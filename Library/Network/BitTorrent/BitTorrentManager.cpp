// Jon Bellamy 10/01/2010



//////////////////////////////////////////////////////////////////////////
// Global TODO's

// * Queued state
// * Reserve a new connection slot per torrent to prevent starving
// * Main listview tag to hold meta data is not working correctly, see RefreshView() in mainform.cs, we are fetching everytime for now as a work around.

// Transport encryption connection failure conditions (bottom of doc), some are done.
// Some DHT msgs
// Hash checks on pieces should not be atomic
// Tab flicker
// Clear all tabs on torrent selection change
// Multi select torrents 
// Run every class off of a single timer, located in cBitTorrentManager



#include "BitTorrentManager.h"


#include <stdio.h>
#include <assert.h>

#include "TinyXml/tinyxml.h"
#include "OpenSSL/sha.h"

#include "File/FileHelpers.h"
#include "Network/NetworkAdaptorList.h"
#include "Network/Upnp/UpnpIgd.h"
#include "Network/BitTorrent/dht/Dht.h"
#include "Network/BitTorrent/dht/DhtPacketLog.h"
#include "Network/BitTorrent/BitTorrentMessages.h"



using namespace net;


cBitTorrentManager gBtManager;
cBitTorrentManager& BitTorrentManager()
{
	return gBtManager;
}// END BitTorrentManager



cBitTorrentManager::cBitTorrentManager()
: mState(STOPPED)
, mNextTorrentId(100)
, mTimer(cTimer::STOPWATCH)
, mDownloadBandwidthPeak(0.0f)
, mUploadBandwidthPeak(0.0f)
, mListenPort(DEFAULT_LISTEN_PORT)
{
}// END cBitTorrentManager



cBitTorrentManager::~cBitTorrentManager()
{
}// END ~cBitTorrentManager



void cBitTorrentManager::Init(const char* appDataFolder)
{
	mState = INITIALIZING;

	// This only prevents the actual printf call, our Printf's are still sent to the output window, this is to speed up debug builds!
	gSuppressPrintf = true;

	// Set the maximum number of open files in the crt to the largest possible value 2048
	int ret = _setmaxstdio(2048);
	assert(ret == 2048);

	UpnpIgd().Init();
	if(UpnpIgd().FoundIgd())
	{
		UpnpIgd().ListExistingPortMappings();
	}

	mAppDataFolder = appDataFolder;
	mAppDataFolder += "\\Meerkat";
	mPeerLogFolder = mAppDataFolder + TORRENT_PACKET_DUMP_FOLDER;
	mDhtLogFolder = mAppDataFolder + DHT_PACKET_DUMP_FOLDER;
	Printf("App data folder: %s\n", mAppDataFolder.c_str());
	FileHelpers::MakeAllDirs(mAppDataFolder.c_str());
	FileHelpers::MakeAllDirs(mPeerLogFolder.c_str());
	FileHelpers::MakeAllDirs(mDhtLogFolder.c_str());

	cEncryptedTorrentConnection::InitCrypto();

	mTimer.Start();

	mOptions.SetDefault();
}// END Init




void cBitTorrentManager::DeInit()
{
	// Delete port mapping
	if(UpnpIgd().FoundIgd())
	{
		UpnpIgd().DeletePortMapping(mListenPort, cUpnpIgd::TCP);
	}

	mTorrentAddedHandlers.Clear();
	mTorrentRemovedHandlers.Clear();

	SaveState();
	RemoveAllTorrents();
	while(mTorrents.size() > 0)
	{
		ManagedTorrent& mt = mTorrents.back();	
		delete mt.mTorrent;
		mt.mTorrent = NULL;
		mTorrents.pop_back();
	}

	mAnnounceManager.Stop();

#if USE_DHT
	if(DhtTaskManager().Running())
	{
		DhtTaskManager().StopDht();
	}
#endif

	cEncryptedTorrentConnection::DeInitCrypto();

	mState = STOPPED;
}// END DeInit



// returns the torrents uid
TorrentHandle cBitTorrentManager::AddTorrent(const char* szTorrentFile, const char* rootDir)
{
	ManagedTorrent mt;
	mt.mHandle = ++mNextTorrentId;
	mt.mTorrent = new cBitTorrent(szTorrentFile, rootDir);
	if(mt.mTorrent->Parse() == false)
	{
		Printf("BitTorrent: Failed to add torrent %s\n", szTorrentFile);
		delete mt.mTorrent;
		return INVALID_TORRENT_HANDLE;
	}

	if(NumTorrentsDownloading() >= NUM_CONCURRENT_TORRENTS_FOR_QUEUED_TO_START)
	{
		mt.mTorrent->PostLoadState(cBitTorrent::QUEUED);
	}
	else
	{
		mt.mTorrent->PostLoadState(cBitTorrent::PEER_MODE);
	}

	// Calculate the stream encryption hash for this torrent so incoming crypto connections can find it
	u8 hashBuf[24];
	memcpy(hashBuf, "req2", 4);
	memcpy(hashBuf+4, mt.mTorrent->InfoHash(), 20);
	SHA1(hashBuf, 24, mt.mReq2InfoHash);
	
	mTorrents.push_back(mt);

	mt.mTorrent->AddDownloadCompleteCallback(TorrentDownloadCompleteHandler, NULL);

	// Fire callbacks
	for(u32 i=0; i < mTorrentAddedHandlers.Size(); i++)
	{
		TorrentAddedCallbackSet::sCallback sCb = mTorrentAddedHandlers.GetCallback(i);
		sCb.mCbFn(mt.mHandle, NULL);
	}

#if USE_DHT
	cDhtResourceId resourceId(mt.mTorrent->InfoHashAsString());
	mt.mpDhtGetPeersTask =  BitTorrentManager().DhtTaskManager().GetPeers(cDhtTaskManager::HIGH_PRIORITY, resourceId, DHT_NUMBER_OF_NODES_PER_GET_PEER_SEARCH, DhtFindPeersCb, reinterpret_cast<void *> (mt.mHandle));
#endif

	return mt.mHandle;
}// END AddTorrent



bool cBitTorrentManager::RemoveTorrent(TorrentHandle handle)
{
	if(GetTorrent(handle) == NULL)
	{
		return false;
	}
	
	StopTorrent(handle);

	// Fire callbacks
	for(u32 i=0; i < mTorrentRemovedHandlers.Size(); i++)
	{
		TorrentRemovedCallbackSet::sCallback sCb = mTorrentRemovedHandlers.GetCallback(i);
		sCb.mCbFn(handle, NULL);
	}

	for(TorrentsVectorIterator iter = mTorrents.begin(); iter != mTorrents.end(); iter++)
	{
		ManagedTorrent& mt = *iter;
		if(mt.mHandle == handle)
		{
#if USE_DHT
			if(mt.mpDhtGetPeersTask)
			{
				BitTorrentManager().DhtTaskManager().DeleteTask(mt.mpDhtGetPeersTask);
			}
#endif

			delete mt.mTorrent;
			mt.mTorrent = NULL;
			mTorrents.erase(iter);
			break;
		}
	}
		
	if(GetTorrent(handle) != NULL)
	{
		assert(0);
		return false;
	}

	return true;
}// END RemoveTorrent



void cBitTorrentManager::RemoveAllTorrents()
{
	while(mTorrents.size() > 0)
	{
		ManagedTorrent& mt = mTorrents.back();	
		RemoveTorrent(mt.mHandle);
	}
}// END RemoveAllTorrents



void cBitTorrentManager::StartTorrent(TorrentHandle handle)
{
	cBitTorrent* pTorrent = GetTorrent(handle);
	if(pTorrent && pTorrent->State() != cBitTorrent::PEER_MODE)
	{
		pTorrent->Start();
	}
}// END StartTorrent



void cBitTorrentManager::StopTorrent(TorrentHandle handle)
{
	cBitTorrent* pTorrent = GetTorrent(handle);
	if(pTorrent && pTorrent->State() != cBitTorrent::STOPPED)
	{
		pTorrent->Stop();
	}
}// END StopTorrent



void cBitTorrentManager::PauseTorrent(TorrentHandle handle)
{
	if(NumTorrentsDownloading() >= NUM_CONCURRENT_TORRENTS_FOR_QUEUED_TO_START)
	{
		cBitTorrent* pTorrent = GetTorrent(handle);
		if(pTorrent)
		{
			pTorrent->Pause();
		}
	}
}// END PauseTorrent



void cBitTorrentManager::Process()
{
	mTimer.Process();

	switch(mState)
	{
	case STOPPED:
		break;

	case INITIALIZING:
		if(UpnpIgd().IsInitialised())
		{
			// Finish up initialisation
			SetIncomingConnectionsListener(DEFAULT_LISTEN_PORT);
			mAnnounceManager.Start();
#if USE_DHT	
			BitTorrentManager().DhtTaskManager().StartDht();
#endif

			LoadState();

			mState = RUNNING;
		}
		break;


	case RUNNING:
		{
			mAnnounceManager.Process();

#if USE_DHT
			DhtTaskManager().Process();
#endif

			ProcessIncomingPeerConnections();
			ProcessNewConnections();

			for(u32 i=0; i < mTorrents.size(); i++)
			{
				mTorrents[i].mTorrent->Process();
			}

			if(NumTorrentsQueued() > 0)
			{
				ProcessQueuedTorrents();
			}
		}
	}
}// END Process



void cBitTorrentManager::ProcessIncomingPeerConnections()
{
	if(mListener.IsOpen() && mListener.IsNewConnectionPending())
	{
		cEncryptedTorrentConnection newConnection(SOCKET_SEND_BUFFER_SIZE, SOCKET_RECV_BUFFER_SIZE);
		cSockAddr newConnectionAddress;

		// don't close the socket when this goes out of scope
		newConnection.SetDontClose(true);

		if (mListener.ProcessIncomingConnection(newConnection, newConnectionAddress))
		{
			newConnection.SetBlockingState(false);

			NewTorrentConnection con(mTimer.ElapsedMs(), newConnection);
			mNewConnections.push_back(con);
		}
	}

#if 1
	// this is a test for the 'cryptoport' announce param
	if(mCryptoListener.IsOpen() && mCryptoListener.IsNewConnectionPending())
	{
		assert(0);

		cEncryptedTorrentConnection newConnection(SOCKET_SEND_BUFFER_SIZE, SOCKET_RECV_BUFFER_SIZE);
		cSockAddr newConnectionAddress;

		// don't close the socket when this goes out of scope
		newConnection.SetDontClose(true);

		if (mCryptoListener.ProcessIncomingConnection(newConnection, newConnectionAddress))
		{
			assert(0);
		}
	}
#endif	
}// END ProcessIncomingPeerConnections



// Peek at new connections handshake so we know which torrent they are for, once we have
// the info-hash forward the connection on and remove it from our list 
void cBitTorrentManager::ProcessNewConnections()
{
	// timeout
	for(NewConnectionIterator i = mNewConnections.begin(); i != mNewConnections.end(); i++)
	{
		(*i).mSocket.Process();

		// TODO : this timeout now needs to cover both the encryption handshake AND the wait for the bittorrent handshake
		if((mTimer.ElapsedMs() - ((*i).mConnectionTime) >= HANDSHAKE_TIMEOUT_MS) ||
			(*i).mSocket.IsOpen() == false)
		{
			mNewConnections.erase(i);
			return;
		}
	}


	for(NewConnectionIterator i = mNewConnections.begin(); i != mNewConnections.end(); i++)
	{
		NewTorrentConnection& connection = *i;

		if(connection.mSocket.ConnectionEstablished())
		{
			const u32 MESSAGE_SIZE = sizeof(cTorrentHandshakeMessage);	
			u32 numBytes = connection.mSocket.BytesPendingOnInputBuffer();
			if(numBytes >= MESSAGE_SIZE)
			{
				cTorrentHandshakeMessage handshake;
				connection.mSocket.Recv(&handshake, MESSAGE_SIZE, true);

				if(!handshake.IsValid())
				{
					Printf("BAD handshake message received on incoming connection, encrypted?\n");
					mNewConnections.erase(i);
					return;
				}

				cBitTorrent* pTorrent = GetTorrent(handshake.mInfoHash);
				if(pTorrent == NULL)
				{
					// handshake for a torrent we don't have (recently closed etc)
					Printf("Connection for a torrent we are not running, ignore.\n");
					mNewConnections.erase(i);
					return;
				}

				// TODO : Need a proper way to allow a good balance of incoming and outgoing connections here !
				if(CanMakePeerConnection(pTorrent, true) == false)
				{
					return;
				}

				if(pTorrent->AmConnectedToPeer(connection.mSocket.AddressRemote()))
				{
					Printf("Incoming connection from a peer we are already connected to, refused.\n");
					connection.mSocket.Close();
					mNewConnections.erase(i);
					return;
				}

				if(pTorrent->ReadyForIncomingConnections() == false)
				{
					Printf("Not ready for connections yet\n");
					mNewConnections.erase(i);
					return;
				}
				
				// handoff peer to torrent
				pTorrent->InsertNewConnection(&connection.mSocket);
				mNewConnections.erase(i);
				return;
			}
		}
	}
}// END ProcessNewConnections



void cBitTorrentManager::DhtFindPeersCb (const cDhtTask* pTask, void* param)
{
	Printf("DhtFindPeersCb\n");
	TorrentHandle handle = reinterpret_cast<TorrentHandle> (param);
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(handle);
	
	// Torrent no longer exists
	if(pTorrent == NULL)
	{
		return;
	}

	const cDhtGetPeersTask* pGetPeersTask = static_cast<const cDhtGetPeersTask*> (pTask);	

	std::vector<cPeerMetaData> addrList;
	u32 numFoundPeers = pGetPeersTask->FoundPeersList().size();
	for(u32 i=0; i < numFoundPeers; i++)
	{
		const cSockAddr& addr = pGetPeersTask->FoundPeersList()[i];
		addrList.push_back(cPeerMetaData(addr));
	}

	if(numFoundPeers > 0)
	{
		pTorrent->PresentPeers(addrList);
	}


	if(pGetPeersTask->SearchHistorySize() >= MAX_DHT_GETPEER_SEARCH_HISTORY_SIZE)
	{
		// Restarting will search nodes we already searched
		BitTorrentManager().DhtTaskManager().RestartTask(pTask);
	}
	else
	{
		// Expanding the search will only introduce new nodes
		u32 count = BitTorrentManager().DhtTaskManager().ExpandTaskSearch(pGetPeersTask, DHT_NUMBER_OF_NODES_PER_GET_PEER_SEARCH);
	}


	// JonB 03/03/2011 : Announce via Dht. 
	// WARNING: Not much thought gone into this! NEED to check that we are not spamming announce msg's to everyone again and again, only announce once to a peer!!!!!!
	//BitTorrentManager().DhtTaskManager().AnnounceResource(cDhtTaskManager::HIGH_PRIORITY, pGetPeersTask->ResourceId(), NULL, NULL);
}// END DhtFindPeersCb



u32 cBitTorrentManager::GetAllTorrentHandles(TorrentHandle* pHandleArray, u32 arraySize)
{
	assert(mTorrents.size() < arraySize);

	u32 numAdded=0;
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		pHandleArray[i] = mTorrents[i].mHandle;
		numAdded++;
	}
	return numAdded;
}// END GetAllTorrentHandles



TorrentHandle cBitTorrentManager::GetHandle(cBitTorrent* pTorrent)
{
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		if(mTorrents[i].mTorrent == pTorrent)
		{
			return mTorrents[i].mHandle;
		}
	}
	return -1;
}// END GetHandle



cBitTorrent* cBitTorrentManager::GetTorrent(TorrentHandle handle)
{
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		if(mTorrents[i].mHandle == handle)
		{
			return mTorrents[i].mTorrent;
		}
	}
	return NULL;
}// END GetTorrent



cBitTorrent* cBitTorrentManager::GetTorrent(const u8* infoHash)
{
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		if(memcmp(mTorrents[i].mTorrent->InfoHash(), infoHash, INFO_HASH_LENGTH) == 0)
		{
			return mTorrents[i].mTorrent;
		}
	}
	return NULL;
}// END GetTorrent



// vile const version of the above function
const cBitTorrent* cBitTorrentManager::GetTorrent(const u8* infoHash) const
{
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		if(memcmp(mTorrents[i].mTorrent->InfoHash(), infoHash, INFO_HASH_LENGTH) == 0)
		{
			return mTorrents[i].mTorrent;
		}
	}
	return NULL;
}// END GetTorrent



const u8* cBitTorrentManager::GetTorrentInfoHashFromReq2Hash(u8* req2Hash)
{
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		if(memcmp(mTorrents[i].mReq2InfoHash, req2Hash, INFO_HASH_LENGTH) == 0)
		{
			return mTorrents[i].mTorrent->InfoHash();
		}
	}
	return NULL;
}// END GetTorrentInfoHashFromReq2Hash




bool cBitTorrentManager::TorrentConnectionsMustBeEncrypted(u8* infohash) const
{
	const cBitTorrent* pTorrent = GetTorrent(infohash);
	if(pTorrent == NULL)
	{
		return false;
	}
	if(Options().AllConnectionsMustBeEncrypted() == false)
	{
		return false;
	}
	// torrents must be encrypted so check & return if this one has an exemption set
	return pTorrent->ConnectionsMustBeEncrypted();
}// END TorrentConnectionsMustBeEncrypted



void cBitTorrentManager::OnOptionsUdated()
{
	if(DhtTaskManager().Running() != Options().UseDht())
	{
		if(Options().UseDht())
		{
			DhtTaskManager().StartDht();
		}
		else
		{
			DhtTaskManager().StopDht();
		}
	}
}// END OnOptionsUdated



u32 cBitTorrentManager::DownloadRate() const
{
	u32 rate=0;
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		rate += mTorrents[i].mTorrent->DownloadRate();
	}

	float f = (rate / 1024.0f);
	if(f > mDownloadBandwidthPeak)
	{
		mDownloadBandwidthPeak = f;
	}

	return rate;
}// END DownloadRate



u32 cBitTorrentManager::UploadRate() const
{
	u32 rate=0;
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		rate += mTorrents[i].mTorrent->UploadRate();
	}

	float f = (rate / 1024.0f);
	if(f > mUploadBandwidthPeak)
	{
		mUploadBandwidthPeak = f;
	}

	return rate;
}// END UploadRate



void cBitTorrentManager::SetIncomingConnectionsListener(u16 port)
{
	// Delete old port mapping
	if(UpnpIgd().FoundIgd())
	{
		UpnpIgd().DeletePortMapping(mListenPort, cUpnpIgd::TCP);
	}

	if(port == mListenPort && 
	   mListener.IsOpen())
	{
		return;
	}	

	// TODO : This should probably re-announce the new port to the trackers

	if(mListener.IsOpen())
	{
		mListener.Close();
	}
	
	mListenPort = port;

	// Add new port mapping
	if(UpnpIgd().FoundIgd())
	{
		if(UpnpIgd().ExistingPortMapping(mListenPort, cUpnpIgd::TCP))
		{
			Printf("BitTorrent: WARNING - Port mapping already exists for port %u. Deleting and redirecting to us.\n", mListenPort);
			UpnpIgd().DeletePortMapping(mListenPort, cUpnpIgd::TCP);
		}
		UpnpIgd().AddPortMapping(mListenPort, mListenPort, cUpnpIgd::TCP, "Meerkat_Port");
	}

	cSockAddr addr(NetworkAdaptorList().GetAdaptorIp(0), mListenPort);
	mListener.OpenAndListen(addr, false);
	

/*
	if(mCryptoListener.IsOpen())
	{
		mCryptoListener.Close();
	}
	cSockAddr addrCrypto(NetworkAdaptorList().GetAdaptorIp(0), mListenPort);
	mCryptoListener.OpenAndListen(addrCrypto, false);
*/
}// END SetIncomingConnectionsListener



// Used to limit peer connections across all torrent
u32 cBitTorrentManager::GlobalNumberOfPeerConnections() const
{
	u32 count=0;
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		count += mTorrents[i].mTorrent->NumberOfPeerConnections();
	}
	return count;
}// END GlobalNumberOfPeerConnections



bool cBitTorrentManager::CanMakePeerConnection(cBitTorrent* pTorrent, bool existingConnection) const
{
	// If the connection is already open then we don't do the half open check
	if(existingConnection == false)
	{
		// Until vista sp2 there is a hard limit of 10 half open tcp connections at once
		u32 halfOpen = 0;
		for(u32 i=0; i < mTorrents.size(); i++)
		{
			const ManagedTorrent mt = mTorrents[i];
			const cBitTorrent* pTorrent = mt.mTorrent;
			assert(pTorrent);

			halfOpen += pTorrent->NumberOfHalfOpenConnections();
		}

		assert(PLATFORM_WIN32);
		OSVERSIONINFO osvi;
		memset(&osvi, 0, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);
		if( osvi.dwMajorVersion >= 6 ||
			(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1))
		{
			// Windows7 or later
			// Theres no limit as such but we enforce one to keep things sane
			if (halfOpen >= MAX_HALF_OPEN_CONNECTIONS)
			{
				return false;
			}
		}
		else
		{
			// Vista SP2 or earlier
			// The hard limit is 10 
			if (halfOpen >= MAX_HALF_OPEN_CONNECTIONS_PRE_WIN7)
			{
				return false;
			}	
		}
	}


	if(pTorrent->NumberOfPeerConnections() < 5)
	{
		return true;
	}

	// TODO?
//	if(pTorrent->NumberOfIncomingConnections() < x)
//	{
//	}

	return GlobalNumberOfPeerConnections() < GLOBAL_MAX_PEER_CONNECTIONS;
}// END CanMakePeerConnection



void cBitTorrentManager::OnPeerConnection(cBitTorrent* pTorrent, const cSockAddr& addr)
{
	ASSERT_MSG(pTorrent, "no torrent");
	
	//mGlobalPeerConnections++;

	TorrentHandle handle = GetHandle(pTorrent);

	// Fire callbacks
	for(u32 i=0; i < mPeerConnectedHandlers.Size(); i++)
	{
		PeerConnectedCallbackSet::sCallback sCb = mPeerConnectedHandlers.GetCallback(i);
		u32 ip = addr.Ip().AsU32();
		sCb.mCbFn(handle, ip, addr.Port(), sCb.mParam);
	}
}// END OnPeerConnection



void cBitTorrentManager::OnPeerDiconnection(cBitTorrent* pTorrent, const cSockAddr& addr)
{
	ASSERT_MSG(pTorrent, "no torrent");

	//mGlobalPeerConnections--;

	TorrentHandle handle = GetHandle(pTorrent);

	// Fire callbacks
	for(u32 i=0; i < mPeerDisconnectedHandlers.Size(); i++)
	{
		PeerDisconnectedCallbackSet::sCallback sCb = mPeerDisconnectedHandlers.GetCallback(i);
		sCb.mCbFn(handle, addr.Ip().AsU32(), addr.Port(), sCb.mParam);
	}
}// END OnPeerDiconnection



u32 cBitTorrentManager::NumTorrentsQueued() const
{
	u32 numTorrentsQueued = 0;
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		cBitTorrent* pTorrent = mTorrents[i].mTorrent;
		if(pTorrent->State() == cBitTorrent::QUEUED)
		{
			numTorrentsQueued++;
		}
	}
	return numTorrentsQueued;
}// END NumTorrentsQueued



u32 cBitTorrentManager::NumTorrentsDownloading() const
{
	u32 numTorrentsDownloading = 0;
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		cBitTorrent* pTorrent = mTorrents[i].mTorrent;
		if(pTorrent->State() == cBitTorrent::PEER_MODE ||
		   (pTorrent->State() == cBitTorrent::CREATE_FILES && pTorrent->PostLoadState() == cBitTorrent::PEER_MODE) )
		{
			numTorrentsDownloading++;
		}
	}
	return numTorrentsDownloading;
}// END NumTorrentsDownloading



void cBitTorrentManager::ProcessQueuedTorrents()
{
	if( NumTorrentsQueued() > 0 && 
		NumTorrentsDownloading() < NUM_CONCURRENT_TORRENTS_FOR_QUEUED_TO_START )
	{
		for(u32 i=0; i < mTorrents.size(); i++)
		{
			cBitTorrent* pTorrent = mTorrents[i].mTorrent;

			if(pTorrent->State() == cBitTorrent::QUEUED)
			{
				pTorrent->Start();
				return;
			}
		}
	}
}// END ProcessQueuedTorrents



void cBitTorrentManager::TorrentDownloadCompleteHandler(cBitTorrent* pTorrent, void* pParam)
{
	//BitTorrentManager().ProcessQueuedTorrents();
}// END TorrentDownloadCompleteHandler



void cBitTorrentManager::SaveTrackerCache()
{
	//for()
	//{
		// If we have had a response from this tracker in this session, then it can go into the tracker cache
		// WriteLine
	//}
}// END SaveTrackerCache



void cBitTorrentManager::LoadTrackerCache()
{
	//for()
	//{
	// If we have had a response from this tracker in this session, then it can go into the tracker cache
	// ReadLine
	//}
}// END LoadTrackerCache



bool cBitTorrentManager::SaveState()
{
	char szTmp[128];

	TiXmlDocument doc;
	TiXmlDeclaration xmlDeclarationNode("1.0", "", "");    // <?xml version='1.0'?>
	doc.InsertEndChild(xmlDeclarationNode);    

	TiXmlElement root(XML_ROOT);

	TiXmlElement optionsRootElement(XML_OPTIONS_ROOT);
	
	// Encrypt connections option
	TiXmlElement optionEncrypt(XML_APP_OPTION_ALLOW_UNSECURE_CONNECTIONS);
	optionEncrypt.SetAttribute(XML_OPTION_VALUE, Options().AllConnectionsMustBeEncrypted());
	optionsRootElement.InsertEndChild(optionEncrypt);
	
	// Use dht
	TiXmlElement optionUseDht(XML_APP_OPTION_DHT_ENABLED);
	optionUseDht.SetAttribute(XML_OPTION_VALUE, Options().UseDht());
	optionsRootElement.InsertEndChild(optionUseDht);

	// Use trackers
	TiXmlElement optionUseTrackers(XML_APP_OPTION_TRACKERS_ENABLED);
	optionUseTrackers.SetAttribute(XML_OPTION_VALUE, Options().UseTrackers());
	optionsRootElement.InsertEndChild(optionUseTrackers);

	// Check for latest build
	TiXmlElement optionCheckBuild(XML_APP_OPTION_CHECK_BUILD);
	optionCheckBuild.SetAttribute(XML_OPTION_VALUE, Options().CheckForLatestBuild());
	optionsRootElement.InsertEndChild(optionCheckBuild);

	// Listen Port
	sprintf(szTmp, "%u", mListenPort);
	TiXmlElement optionListenPort(XML_APP_OPTION_LISTEN_PORT);
	optionListenPort.SetAttribute(XML_OPTION_VALUE, szTmp);
	optionsRootElement.InsertEndChild(optionListenPort);

	// Stop on completion
	TiXmlElement optionStopOnCompletion(XML_APP_OPTION_STOP_ON_COMPLETION);
	optionStopOnCompletion.SetAttribute(XML_OPTION_VALUE, Options().StopOnCompletion());
	optionsRootElement.InsertEndChild(optionStopOnCompletion);

	// Max upload rate
	sprintf(szTmp, "%u", Options().MaxUploadRate() / 1024);
	TiXmlElement optionMaxUploadRate(XML_APP_OPTION_MAX_UPLOAD_RATE);
	optionMaxUploadRate.SetAttribute(XML_OPTION_VALUE, szTmp);
	optionsRootElement.InsertEndChild(optionMaxUploadRate);

	root.InsertEndChild(optionsRootElement);


	TiXmlElement itemsRoot(XML_TORRENTS_ROOT);
	for(u32 i=0; i < mTorrents.size(); i++)
	{
		const ManagedTorrent mt = mTorrents[i];
		const cBitTorrent* pTorrent = mt.mTorrent;
		assert(pTorrent);

		TiXmlElement item(XML_ITEM_ID);
		item.SetAttribute(XML_ITEM_NAME, pTorrent->FileName().c_str());
		item.SetAttribute(XML_ITEM_ROOT, pTorrent->FileSet().RootFolder().c_str());
		
		switch(pTorrent->State())
		{
		case cBitTorrent::PEER_MODE:
		case cBitTorrent::SEED_MODE:
			item.SetAttribute(XML_ITEM_STATE, TORRENT_STATE_STARTED);
			break;

		case cBitTorrent::STOPPED:
		case cBitTorrent::RECHECKING:
		case cBitTorrent::CREATE_FILES:
			item.SetAttribute(XML_ITEM_STATE, TORRENT_STATE_STOPPED);
			break;

		case cBitTorrent::QUEUED:
			item.SetAttribute(XML_ITEM_STATE, TORRENT_STATE_QUEUED);
			break;

		default:
			assert(0);
		}


		// Store partially downloaded pieces
		const cBitTorrent::TorrentPieceVector& queuedPieces = pTorrent->DownloadingPiecesArray();
		bool havePartialPieces = false;
		if(queuedPieces.empty() == false)
		{
			TiXmlElement piecesRoot(XML_ITEM_PARTIAL_PIECES);
		
			for(u32 i=0; i < queuedPieces.size(); i++)
			{
				const cTorrentPiece* piece = queuedPieces[i];

				if(piece->NumberOfCompletedBlocks() > 0)
				{
					havePartialPieces = true;
				}
				else
				{
					continue;
				}

				TiXmlElement pieceNode(XML_ITEM_PIECE);

				int pieceNumber = static_cast<int> (piece->PieceNumber());
				int numberOfBlocks = static_cast<int> (piece->NumberOfBlocks());
				int pieceSize = static_cast<int> (piece->PieceSize());
				std::string b64PieceBitfield = piece->BitfieldBase64();

				pieceNode.SetAttribute(XML_ITEM_PIECE_NUMBER, pieceNumber);
				pieceNode.SetAttribute(XML_ITEM_PIECE_NUMBER_OF_BLOCKS, numberOfBlocks);
				pieceNode.SetAttribute(XML_ITEM_PIECE_SIZE, pieceSize);
				pieceNode.SetAttribute(XML_ITEM_PIECE_BITFIELD, b64PieceBitfield.c_str());

				piecesRoot.InsertEndChild(pieceNode);
			}

			if(havePartialPieces)
			{
				item.InsertEndChild(piecesRoot);
			}
		}



		itemsRoot.InsertEndChild(item);
	}
	root.InsertEndChild(itemsRoot);    

	doc.InsertEndChild(root);
	doc.SaveFile(std::string(AppDataFolder() + "\\" + XML_FILENAME).c_str());

	SaveTrackerCache();

	return true;
}// END SaveState



bool cBitTorrentManager::LoadState()
{
	TiXmlDocument readDoc;
	bool docLoaded = readDoc.LoadFile(std::string(AppDataFolder() + "\\" + XML_FILENAME).c_str());

	if(!docLoaded)
	{
		return false;
	}

	TiXmlElement* pRoot = readDoc.RootElement();
	if(!pRoot || pRoot->Value() != std::string(XML_ROOT))
	{
		assert(0);
		return false;
	}

	TiXmlElement* pAppOptionsRootElement = pRoot->FirstChildElement(XML_OPTIONS_ROOT);
	if(pAppOptionsRootElement && pAppOptionsRootElement->Value() == std::string(XML_OPTIONS_ROOT))
	{
		// TODO : if you change options in the xml to a string, you can get stack corruption here !!!

		const char* szValue;
		int attrValue=-1;

		// Encrypt connections option
		TiXmlElement* pOptionEncrypt = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_ALLOW_UNSECURE_CONNECTIONS);
		if(pOptionEncrypt)
		{
			szValue = pOptionEncrypt->Attribute(XML_OPTION_VALUE, &attrValue);
			if(attrValue == 0 || attrValue == 1)
			{
				Options().AllConnectionsMustBeEncrypted(attrValue!=0);
			}
		}


		// Use dht
		TiXmlElement* pOptionUseDht = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_DHT_ENABLED);
		if(pOptionUseDht)
		{
			szValue = pOptionUseDht->Attribute(XML_OPTION_VALUE, &attrValue);
			if(attrValue == 0 || attrValue == 1)
			{
				Options().UseDht(attrValue!=0);
			}
		}

		// Use trackers
		TiXmlElement* pOptionUseTrackers = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_TRACKERS_ENABLED);
		if(pOptionUseTrackers)
		{
			szValue = pOptionUseTrackers->Attribute(XML_OPTION_VALUE, &attrValue);
			if(attrValue == 0 || attrValue == 1)
			{
				Options().UseTrackers(attrValue!=0);
			}
		}


		// Check for latest build
		TiXmlElement* pOptionCheckBuild = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_CHECK_BUILD);
		if(pOptionCheckBuild)
		{
			szValue = pOptionCheckBuild->Attribute(XML_OPTION_VALUE, &attrValue);
			if(attrValue == 0 || attrValue == 1)
			{
				Options().CheckForLatestBuild(attrValue!=0);
			}
		}

		// Listen Port
		TiXmlElement* pListenPortNode = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_LISTEN_PORT);
		if(pListenPortNode)
		{
			szValue = pListenPortNode->Attribute(XML_OPTION_VALUE, &attrValue);
			u16 port = static_cast<u16> (attrValue);
			SetIncomingConnectionsListener(port);
		}


		// Stop on completion
		TiXmlElement* pStopOnCompletion = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_STOP_ON_COMPLETION);
		if(pStopOnCompletion)
		{
			szValue = pStopOnCompletion->Attribute(XML_OPTION_VALUE, &attrValue);
			if(attrValue == 0 || attrValue == 1)
			{
				Options().StopOnCompletion(attrValue!=0);
			}
		}


		// Max upload rate
		TiXmlElement* pOptionMaxUploadRate = pAppOptionsRootElement->FirstChildElement(XML_APP_OPTION_MAX_UPLOAD_RATE);
		if(pOptionMaxUploadRate)
		{
			int val;
			szValue = pOptionMaxUploadRate->Attribute(XML_OPTION_VALUE, &val);
			if(val < 5)
			{
				val = 5;
			}
			Options().MaxUploadRate(val * 1024);
		}

		OnOptionsUdated();
	}



	TiXmlElement* pItemsRoot = pRoot->FirstChildElement(XML_TORRENTS_ROOT);
	if(!pItemsRoot || pItemsRoot->Value() != std::string(XML_TORRENTS_ROOT))
	{
		assert(0);
		return false;
	}
	TiXmlElement* pItemElement = pItemsRoot->FirstChildElement(XML_ITEM_ID);
	for(; pItemElement != NULL; pItemElement = pItemElement->NextSiblingElement(XML_ITEM_ID))
	{
		if(pItemElement->Value() != std::string(XML_ITEM_ID))
		{
			assert(0);
			continue;
		}

		const char* szFileName = pItemElement->Attribute(XML_ITEM_NAME);
		const char* szRootDir = pItemElement->Attribute(XML_ITEM_ROOT);
		std::string strState = pItemElement->Attribute(XML_ITEM_STATE);
		if(szFileName == NULL || szRootDir == NULL)
		{
			assert(0);
			continue;
		}

		
		// TODO : check this torrent isn't already there

		TorrentHandle handle = AddTorrent(szFileName, szRootDir);
		if(handle == INVALID_TORRENT_HANDLE)
		{
			continue;
		}


		// Load partially downloaded pieces
		TiXmlElement* piecesRoot = pItemElement->FirstChildElement(XML_ITEM_PARTIAL_PIECES);
		if(piecesRoot)
		{
			TiXmlElement* pieceNode = piecesRoot->FirstChildElement(XML_ITEM_PIECE);
			while(pieceNode)
			{
				int pieceNumber=-1;
				int numberOfBlocks=-1;
				int pieceSize=-1;
				std::string b64PieceBitfield;

				pieceNode->Attribute(XML_ITEM_PIECE_NUMBER, &pieceNumber);
				pieceNode->Attribute(XML_ITEM_PIECE_NUMBER_OF_BLOCKS, &numberOfBlocks);
				pieceNode->Attribute(XML_ITEM_PIECE_SIZE, &pieceSize);
				const char* base64PieceBitfield = pieceNode->Attribute(XML_ITEM_PIECE_BITFIELD);

				// TODO : The check below will fail for the final piece, not a big deal but we should sort it.

				// Check numblocks, piece size etc then insert
				if( pieceNumber >= 0 && pieceNumber < static_cast<int> (GetTorrent(handle)->FileSet().NumberOfPieces()) &&
					numberOfBlocks == (pieceSize / BLOCK_LENGTH))
				{
					// Insert the partial piece
					GetTorrent(handle)->InsertPartiallyDownloadedPiece(pieceNumber, base64PieceBitfield);
				}

				pieceNode = pieceNode->NextSiblingElement(XML_ITEM_PIECE);
			}
		}


		cBitTorrent* pTorrent = GetTorrent(handle);

		if(pTorrent)
		{
			if(strState == TORRENT_STATE_STARTED)
			{
				pTorrent->PostLoadState(cBitTorrent::PEER_MODE);
			}
			else if (strState == TORRENT_STATE_STOPPED)
			{
				pTorrent->PostLoadState(cBitTorrent::STOPPED);
			}
			else if (strState == TORRENT_STATE_QUEUED)
			{
				pTorrent->PostLoadState(cBitTorrent::QUEUED);
			}
			else
			{
				assert(0);
				continue;
			}
		}
	}

	LoadTrackerCache();

	return true;
}// END LoadState


cAnnounceManager& cBitTorrentManager::AnnounceManager()
{
	return mAnnounceManager;
}// END AnnounceManager