// Jon Bellamy 20/02/2009


#include "BitTorrent.h"


#include <stdio.h>
#include <assert.h>
#include <algorithm>


#include "File/file.h"
#include "File/FileHelpers.h"
#include "openssl/sha.h"
#include "General/Rand.h"
#include "General/Endianness.h"
#include "Network/Url.h"
#include "Network/NetworkAdaptorList.h"
#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/BitTorrent/BitTorrentMessages.h"
#include "Network/BitTorrent/BEncoding/BencodedString.h"
#include "Network/BitTorrent/BEncoding/BencodedInt.h"
#include "Network/BitTorrent/BEncoding/BencodedList.h"
#include "Network/BitTorrent/MessagePump.h"


using namespace net;


#define LOAD_DL_STATE 1
#define DUPE_DL_CHECK 1

#define BAD_PIECES_CHECK 0


cBitTorrent::cBitTorrent(const char* torrentMetaFile, const char* rootDir)
: mMetaFileName(torrentMetaFile)
, mTimer(cTimer::STOPWATCH)
, mTimeStarted(0)
, mConnectionsMustBeEncrypted(true)
, mFileSet(mMetaFile, rootDir)
, mState(STOPPED)
, mIsEndGame(false)
, mPostLoadState(STOPPED)
, mNextPieceToDownload(0)
, mNumberOfPiecesDownloading(0)
, mLastChokeProcess(0)
, mLastRollingChokeProcess(0)
, mRollingUnChoke(INVALID_PEER_INDEX)
, mEventFlags(0)
, mNextPieceForRecheck(0)
, mBytesDownloadedInThisSession(0)
, mBytesUploadedInThisSession(0)
{
	Printf("BitTorrent: New torrent created : %s\n", torrentMetaFile);

	// TODO: Dht uses an app version string in the KRPC messages (currently set to MK01). We need to roll the ID below and that ID into a global, probably using the .net version number!

	// Azureus-style uses the following encoding: '-', two characters for client id, four ascii digits for version number, '-', followed by random numbers. For example: '-AZ2060-'... 
	char id[32];
	sprintf(id, "-MK0016-%.6u%.6u", Rand32(999999), Rand32(999999));
	memcpy(mPeerId, id, PEER_ID_LENGTH);

	mTimer.Start();
}// END cBitTorrent



cBitTorrent::~cBitTorrent()
{
	Stop();

	for(TorrentPieceVectorIterator iter = mDownloadingPieces.begin(); iter != mDownloadingPieces.end(); iter++)
	{
		cTorrentPiece* pPiece = *iter;
		delete pPiece;
	}
	mDownloadingPieces.clear();
}// END ~cBitTorrent



bool cBitTorrent::Start()
{
	if(State() != STOPPED && State() != QUEUED)
	{
		return false;
	}

	cTorrentPacketLog::ClearLogFolder(BitTorrentManager().PeerLogFolder());

	ClearAllEventFlags();

	mTimeStarted = Time();

	BitTorrentManager().AnnounceManager().RegisterTorrentForTrackerUpdates(this);

	if(AllPiecesDownloaded())
	{
		mState = SEED_MODE;
	}
	else
	{
		mState = PEER_MODE;
	}

	return true;
}// END Start



void cBitTorrent::Stop()
{
	if(State() == CREATE_FILES)
	{
		FileSet().CancelFileCreation();
	}

	mHttpClient.Close();

	BitTorrentManager().AnnounceManager().UnregisterTorrentForTrackerUpdates(this);

	mBytesDownloadedInThisSession = 0;
	mBytesUploadedInThisSession = 0;

	FileSet().Flush();
	//FileSet().CloseAll();

	while(mPeerList.empty() == false)
	{
		DisconnectFromPeer(0);
	}

	ClearPeerList();
	mIpPool.clear();
	mState = STOPPED;
}// END Stop



void cBitTorrent::Pause()
{
	Stop();
	mState = QUEUED;
}// END Pause



bool cBitTorrent::Parse()
{
	if(!mMetaFile.Parse(mMetaFileName.c_str()))
	{
		return false;
	}	

	mState = CREATE_FILES;

	if(!mFileSet.OpenAllFiles(true))
	{
		return false;
	}

	mPiecesBitfield.Resize(mFileSet.NumberOfPieces(), 1);	

	Printf("BitTorrent: Torrent %s\n", mMetaFileName.c_str());
	Printf("%d pieces\n", mFileSet.NumberOfPieces());
	Printf("%d piece size\n", mFileSet.PieceSize());
	Printf("%d final piece size\n", mFileSet.FinalPieceSize());


	// calc the hash of the info field, this is the torrents uid
	// SHA1() computes the SHA-1 message digest of the n bytes at d and places it in md (which must have space for SHA_DIGEST_LENGTH == 20 bytes of output)
	const std::string* infoField = mMetaFile.RootDictionary().GetValueRawData("info");
	SHA1(reinterpret_cast<const u8*>(infoField->c_str()), infoField->size(), mInfoHash);
	std::string strHash;
	strHash.insert(0, (char*)(&mInfoHash[0]), SHA_DIGEST_LENGTH);


	//const cBencodedString* bStr = static_cast<const cBencodedString*> (mMetaFile.RootDictionary().GetValue("nodes"));

#if LOAD_DL_STATE
	std::string fn = mMetaFileName + ".mk";
	if(FileHelpers::FileExists(fn.c_str()))
	{
		if(mPiecesBitfield.LoadFromDisk(fn.c_str()))
		{
			Printf("BitTorrent: Loaded bitfield. Pieces - %d / %d\n", NumberOfPiecesDownloaded(), FileSet().NumberOfPieces());
		}
	}
#endif


#if BAD_PIECES_CHECK
	Recheck();
#endif // BAD_PIECES_CHECK

	BitTorrentManager().AnnounceManager().RegisterTorrentForTrackerUpdates(this);
	PopulateAnnounceManager();

	return true;
}// END Parse



void cBitTorrent::Process()
{
	mTimer.Process();
	mHttpClient.Process();

	CacheIsEndGame();

	for(u32 i=0; i < mPeerList.size(); i++)
	{
		mPeerList[i]->Process();
	}


#if PRINT_BANDWIDTH
	static u32 mLastBandwidthPrint=0;
	if(mTimer.ElapsedMs() - mLastBandwidthPrint >= 5000)
	{
		mLastBandwidthPrint = mTimer.ElapsedMs();
		Printf("Download rate = %.2f kB/s\n", float(DownloadRate() / 1024.0f));
		Printf("Upload rate = %.2f kB/s\n", float(UploadRate() / 1024.0f));
		
		// TODO : format this
		s64 seconds = Eta();
		u32 hours = u32(seconds / (60*60));
		u32 minutes = u32((seconds - (hours * (60*60))) / 60);
		Printf("ETA %uh %.2um\n", hours, minutes);
	}
#endif

	switch(State())
	{
	case STOPPED:
	case QUEUED:
		break;

	case CREATE_FILES:
		ProcessState_CreateFiles();
		break;

	case PEER_MODE:
		ProcessState_PeerMode();
		break;

	case SEED_MODE:
		ProcessState_SeedMode();
		break;

	case RECHECKING:
		ProcessState_Rechecking();
		break;

	default:
		assert(0);
	}
}// END Process



void cBitTorrent::Recheck()
{
	mState = RECHECKING;
	mPiecesBitfield.Zero();
	mNextPieceForRecheck = 0;
}// END Recheck



void cBitTorrent::PopulateAnnounceManager()
{
	const cBencodedString* bStr=NULL;
	const cBencodedList* announceList = static_cast<const cBencodedList*> (MetaFile().RootDictionary().GetValue("announce-list"));
	if(announceList)
	{
		for(u32 i=0; i < announceList->NumElements(); i++)
		{
			const cBencodedList* subList = dynamic_cast<const cBencodedList*> (announceList->GetElement(i));
			if(subList)
			{
				bStr = dynamic_cast<const cBencodedString*> (subList->GetElement(0));
			}
			else
			{
				bStr = dynamic_cast<const cBencodedString*> (announceList->GetElement(i));
			}

			if(bStr == NULL)
			{
				return;
			}
			

			cUrl url(bStr->Get().c_str());
			if(url.IsValid() == false)
			{
				continue;
			}
			BitTorrentManager().AnnounceManager().AddAnnounceUrl(url);
		}
	}
	else
	{
		bStr = static_cast<const cBencodedString*> (mMetaFile.RootDictionary().GetValue("announce"));
		if(bStr)
		{
			cUrl url(bStr->Get().c_str());
			BitTorrentManager().AnnounceManager().AddAnnounceUrl(url);
			//Printf("announce %s\n", bStr->Get().c_str());
		}
	}
}// END PopulateAnnounceManager




bool cBitTorrent::ConnectToPeer(const cPeerMetaData& peer)
{
	// counting on number of pieces fitting into a u32 here
	cTorrentPeer* pPeer = new cTorrentPeer(peer.Addr(), static_cast<u32>(mFileSet.NumberOfPieces()), mTimer, *this);

	mPeerList.push_back(pPeer);	

	return pPeer->OpenConnection();
}// END ConnectToPeer



void cBitTorrent::DisconnectFromPeer(u32 peerPoolIndex)
{
	Printf("BitTorrent: Disconnecting from %s\n", mPeerList[peerPoolIndex]->Address().Ip().AsString());

	// check if we are disconnecting the rolling unchoke
	if(peerPoolIndex == mRollingUnChoke)
	{
		mRollingUnChoke = INVALID_PEER_INDEX;
	}

	if(mPeerList[peerPoolIndex]->HandshakeReceived())
	{
		BitTorrentManager().OnPeerDiconnection(this, mPeerList[peerPoolIndex]->Addr());
	}

	mPeerList[peerPoolIndex]->LogStandardMessage(mTimer.ElapsedMs(), &(mPeerList[peerPoolIndex]->Address()), cTorrentPacketLog::CLOSE_CONNECTION, PTYPE_SEND);
	mPeerList[peerPoolIndex]->Socket().Close();
	delete mPeerList[peerPoolIndex];
	mPeerList[peerPoolIndex] = NULL;
	mPeerList.erase(mPeerList.begin() + peerPoolIndex);
}// END DisconnectFromPeer


/*
void cBitTorrent::DisconnectFromPeer(cTorrentPeer* pPeer)
{
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if(pPeer == mPeerList[i])
		{
			DisconnectFromPeer(i);
			return;
		}
	}
	assert(0);
}// END DisconnectFromPeer
*/


cPeerMetaData* cBitTorrent::PeerMetaData(const net::cSockAddr& addr)
{
	for(IpListIterator iter = mIpPool.begin(); iter != mIpPool.end(); iter++)
	{
		if((*iter).Addr() == addr)
		{
			return &(*iter);
		}
	}
	return NULL;
}// END PeerMetaData



const cTorrentPeer* cBitTorrent::GetPeer(const cSockAddr& addr) const
{
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		cTorrentPeer* pPeer = mPeerList[i];
		assert(pPeer);
		if(pPeer->Address() == addr)
		{
			return pPeer;
		}
	}
	return NULL;
}// END GetPeer



bool cBitTorrent::AmConnectedToPeer(const net::cSockAddr& sockAddr)	const
{
	return (GetPeer(sockAddr) != NULL);
}// END AmConnectedToPeer



void cBitTorrent::ClearPeerList()
{
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		delete mPeerList[i];
		mPeerList[i] = NULL;
	}
	mPeerList.clear();
}// END ClearPeerList



// The announce manager or DHT has found some peers, add them (look for dupes)
void cBitTorrent::PresentPeers(const std::vector<cPeerMetaData>& peerList)
{
	u32 count=0;
	for(u32 i=0; i < peerList.size(); i++)
	{
		// don't add our own ip. NOT WORKING (PORT TRANSLATION), NEED TO CHECK UID
		if(peerList[i].Addr().Ip() == NetworkAdaptorList().GetAdaptorIp(0))
		{
			continue;
		}

		assert(peerList[i].Addr().Ip() != cIpAddr(127,0,0,1));

		if(PeerMetaData(peerList[i].Addr()) == NULL)
		{
			// new peer
			count++;
			mIpPool.push_back(peerList[i]);
		}
	}

	Printf("BitTorrent: %u new peers found. New ip pool %u\n", count, mIpPool.size());
}// END PresentPeers



u32 cBitTorrent::NumberOfPeerConnections() const
{
	u32 count=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if(mPeerList[i]->Socket().ConnectionEstablished())
		{
			count++;
		}
	}
	return count;
}// END NumberOfPeerConnections



cPeerMetaData* cBitTorrent::NextPeerToConnectTo()
{
#if RANDOM_PEER_CONNECTIONS
	u32 index = Rand32(mIpPool.size());
	IpConstListIterator iter = mIpPool.begin(); 
	for(u32 i=0; i < index; i++)
	{
		iter++;
	}
	return *iter;
#endif


	// TODO : are we in this list ??!!??!


	// the list is sorted so just find the first one in the list we are not connected or connecting to
	for(IpListIterator iter = mIpPool.begin(); iter != mIpPool.end(); iter++)
	{
		if(!AmConnectedToPeer((*iter).Addr()))
		{
			return &(*iter);
		}
	}

	return NULL;
}// END NextPeerToConnectTo



void cBitTorrent::InsertNewConnection(cEncryptedTorrentConnection* pSocket)
{
	cSockAddr newConnectionAddress = pSocket->AddressRemote();

	// don't add our own ip. 
	// TODO : This is pretty backwards, need to stop the connection in the first place!
	if(newConnectionAddress.Ip() == NetworkAdaptorList().GetAdaptorIp(0))
	{
		pSocket->SetDontClose(false);
		assert(0);
		return;
	}

	assert((newConnectionAddress.Ip().GetB1() == 192 && newConnectionAddress.Ip().GetB2() == 168) == false);
	assert(newConnectionAddress.Ip() != cIpAddr(0,0,0,0));
	assert(newConnectionAddress.Ip() != cIpAddr(255, 255, 255, 255));

	if(AmConnectedToPeer(newConnectionAddress))
	{
		assert(0);
		Printf("BitTorrent: Already connected to peer, refusing connection\n");
		pSocket->SetDontClose(false);
		return;
	}

	pSocket->SetBlockingState(false);
	cTorrentPeer* pPeer = new cTorrentPeer(newConnectionAddress, static_cast<u32>(mFileSet.NumberOfPieces()), mTimer, *this);
	mPeerList.push_back(pPeer);
	pPeer->SetConnection(*pSocket, newConnectionAddress);
	pPeer->LogStandardMessage(0, &newConnectionAddress, cTorrentPacketLog::INCOMING_CONNECTION, PTYPE_RECEIVE);
}// END InsertNewConnection



void  cBitTorrent::DisconnectAllUnencryptedPeers()
{
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if(mPeerList[i]->Socket().IsEncryptedConnection() == false)
		{
			// the rest of the peers info will get cleaned up shortly
			mPeerList[i]->Socket().Close();
		}
	}
}// END DisconnectAllUnencryptedPeers



const cTorrentPiece* cBitTorrent::PeerHasAQueuedPieceTheyCanDownload(const cTorrentPeer* pPeer) const
{
	for(u32 i=0; i < mDownloadingPieces.size(); i++)
	{
		assert(mDownloadingPieces[i]->IsValid());

		if(!mDownloadingPieces[i]->AllBlocksPending() && 
		   pPeer->HasPiece(mDownloadingPieces[i]->PieceNumber()))
		{

			return mDownloadingPieces[i];
		}
	}

	return NULL;
}// END PeerHasAQueuedPieceTheyCanDownload



bool cBitTorrent::PeerHasPiecesWeWant(const cTorrentPeer* pPeer) const
{
	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		if(mPiecesBitfield.Get(i) == 0 &&
		   pPeer->HasPiece(i))
		{
			return true;
		}
	}
	return false;
}// END PeerHasPiecesWeWant



// Used for passing data back to frontend app
void cBitTorrent::GetAllConnectedPeers(std::vector<const cTorrentPeer*>& listOut) const
{
	listOut.clear();
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if( mPeerList[i]->Socket().ConnectionEstablished())
		{
			listOut.push_back(mPeerList[i]);
		}
	}
}// END GetAllConnectedPeers
 


cTorrentPiece* cBitTorrent::QueuePieceForDownload(u32 pieceNum)
{
	u32 nextPieceNumber = pieceNum;
	u32 pieceSize = (nextPieceNumber == (FileSet().NumberOfPieces()-1) ? FileSet().FinalPieceSize() : FileSet().PieceSize());
	cTorrentPiece* pNewPiece = new cTorrentPiece(nextPieceNumber, pieceSize);
	mDownloadingPieces.push_back(pNewPiece);
	

#if DUPE_DL_CHECK
	// if its not end game we should never have dupe slots on the dl queue
	if(!IsEndGame())
	{
		for(u32 i=0; i < NumberOfPiecesDownloading(); i++)
		{
			for(u32 j=0; j < NumberOfPiecesDownloading(); j++)
			{
				if(j==i) continue;

				if(mDownloadingPieces[j]->PieceNumber() == mDownloadingPieces[i]->PieceNumber())
				{
					assert(0);			
				}
			}
			
		}
	}
#endif
	return pNewPiece;
}// END QueuePieceForDownload


void cBitTorrent::QueueAllRemainingPieces()
{
	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		if( HavePiece(i) == false &&
			IsPieceQueued(i) == false)
		{
			QueuePieceForDownload(i);
		}
	}
}// END QueueAllRemainingPieces



void cBitTorrent::InsertPartiallyDownloadedPiece(u32 pieceNumber, const std::string& base64Bitfield)
{
	// Check each piece to make sure we don't have it, this can happen if we crashed last time after downloading data and the state xml is out of date.
	if( mPiecesBitfield.Get(pieceNumber))
//		FileSet().IsLocalPieceValid(pieceNumber))
	{
		return;
	}

	cTorrentPiece* pNewQueuedPiece = QueuePieceForDownload(pieceNumber);
	assert(pNewQueuedPiece);
	if(pNewQueuedPiece)
	{
		pNewQueuedPiece->SetBitfieldFromBase64Str(base64Bitfield);
	}
}// END InsertPartiallyDownloadedPiece



void cBitTorrent::ProcessReceivedMessages()
{
	s32 numBytes;

	// pump through all received messages
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		cTorrentPeer* pPeer = mPeerList[i];

		if(pPeer->Socket().ConnectionEstablished() 
			&& pPeer->HandshakeSent()
#if 0
			&& pPeer->Socket().ReadyToUse()
#endif
			)
		{
			bool messageProcessed;
			do
			{
				if(!pPeer->Socket().ConnectionEstablished())
				{
					pPeer->LogNote(mTimer.ElapsedMs(), "Socket is no longer connected, disconnect.");
					DisconnectFromPeer(i);
					return;
				}

				messageProcessed=false;

				numBytes = pPeer->Socket().Recv(mRecvBuffer, RECV_BUFFER_SIZE, true);			

				if(numBytes < 0)
				{
					cPeerMetaData* meta = PeerMetaData(mPeerList[i]->Address());
					if(meta)
					{
						meta->OnFailedConnectionAttempt();
					}

					int err = WSAGetLastError();
					
					char errMsg[512];
					sprintf(errMsg, "Recv Failed. Error No %d\n%s\n", err, strerror(err));
					pPeer->LogNote(mTimer.ElapsedMs(), errMsg);

					DisconnectFromPeer(i);
					return;
				}

				// messages are prefixed with size
				if(numBytes >= static_cast<s32>(cTorrentMessage::HeadSize()))
				{
					// pretty ugly way they chose to do the handshake message, it doesn't have a type field so we have to special case it here
					if(pPeer->HandshakeReceived() == false)
					{		
						const u32 MESSAGE_SIZE = sizeof(cTorrentHandshakeMessage);
						if(numBytes >= MESSAGE_SIZE)
						{
							messageProcessed=true;

							// read the data 
							numBytes = mPeerList[i]->Socket().Recv(mRecvBuffer, MESSAGE_SIZE, false);

							const cTorrentHandshakeMessage* pMsg = reinterpret_cast<const cTorrentHandshakeMessage*> (mRecvBuffer);

							if(!pMsg->IsValid())
							{
								// close connection

								// encrypted???

								// TODO : this is a good assert and we need to solve this but for now lets just drop these connections
								// size is good, must be encrypted
								//assert(0);

								Printf("BitTorrent: BAD handshake message received\n");

								cPeerMetaData* meta = PeerMetaData(pPeer->Address());
								if(meta)
								{
									meta->OnFailedConnectionAttempt();
								}


								mPeerList[i]->LogNote(mTimer.ElapsedMs(), "BAD handshake message received\n");
								DisconnectFromPeer(i);
								return;
							}
							pPeer->OnHandshakeReceived(*pMsg);
							SendBitfieldMessage(*pPeer, mPiecesBitfield);
							
							// We class a connection as open when we have the handshake
							BitTorrentManager().OnPeerConnection(this, pPeer->Addr());
						}
					}
					else
					{
						cTorrentMessage* pMsg = reinterpret_cast<cTorrentMessage*> (mRecvBuffer);


						u32 msgSize;
						memcpy(&msgSize, mRecvBuffer, sizeof(u32));
						endian_swap(msgSize);

						// temp test !!!
						// TODO : this needs wrapping up in an IsValid() check but also is it a bug ??!!?!?!?!
						s32 totalMessageSize = s32(static_cast<s32>(msgSize) + sizeof(msgSize));
						if(totalMessageSize < 0 || totalMessageSize > 1024*65 ||
							(u32)pMsg->Type() > NUM_TORRENT_MESSAGE_TYPES)
						{
							// bullshit size message, encrypted packet?
							// TODO : This REALLY needs fixing

							//assert(0);

							char errMsg[64];
							sprintf(errMsg, "Corrupt Recv, size = %d\n", totalMessageSize);
							mPeerList[i]->LogNote(mTimer.ElapsedMs(), errMsg);

							DisconnectFromPeer(i);
							return;
						}


						// keep alive is a zero sized message
						if(msgSize == 0)
						{
							mPeerList[i]->Socket().Recv(&msgSize, sizeof(u32), false);
							pPeer->OnMessageReceived();
							MessageHandler_KeepAlive(*pPeer, this);
							messageProcessed=true;
							continue;
						}



						// read the data 
						if(numBytes >= s32(static_cast<s32>(msgSize) + sizeof(msgSize)))
						{
							numBytes = mPeerList[i]->Socket().Recv(mRecvBuffer, msgSize + sizeof(msgSize), false);

							// sort out endianess
							pMsg->OnReceive();

							if(numBytes != (pMsg->mSize + sizeof(msgSize)))
							{
								assert(0);

								//char errMsg[64];
								//sprintf(errMsg, "Corrupt Recv, size = %d\n", numBytes)
								//pPeer->LogNote(mTimer.ElapsedMs(), errMsg);

								DisconnectFromPeer(i);
								return;
							}						

							messageProcessed=true;

							pPeer->OnMessageReceived();

							NetMessagePump().DispatchNetMessage(*pPeer, this, *pMsg);

							pPeer->LogStandardMessage(0, pMsg, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_RECEIVE);
						}
					}
				}				
			}
			while(messageProcessed);
		}
	}
}// END ProcessReceivedMessages


void cBitTorrent::ProcessPeerConnections()
{
	u32 i;

	if(BitTorrentManager().CanMakePeerConnection(this, false))
	{
		// make peer connections as slots become available
		if( mIpPool.size() > 0 &&
			mIpPool.size() != mPeerList.size())
			/*mPeerList.size() < MAX_PEER_CONNECTIONS_PER_TORRENT )*/
		{
			// sort the peer list so the best candidate is at the front then connect to the first one we are
			// not connected to
			mIpPool.sort();
			cPeerMetaData* meta = NextPeerToConnectTo();
			if(meta)
			{
				if(!ConnectToPeer(*meta))
				{
					meta->OnFailedConnectionAttempt();
				}
			}
		}
	}

	// remove bad connections
	for(i=0; i < mPeerList.size(); i++)
	{
		cTorrentPeer* pPeer = mPeerList[i];
		assert(pPeer);

		// TODO : can check if the peer has nothing we want as well but not every frame and 
		// watch for no bitfield disconnects !

		if(pPeer && !pPeer->IsPeerUsable())
		{
			cPeerMetaData* meta = PeerMetaData(pPeer->Address());
			if(meta)
			{
				meta->OnFailedConnectionAttempt();
			}

			DisconnectFromPeer(i);
			break;
		}
	}



	// TODO : if we have no peers then schedule another announce in a few minutes


	// send handshake / interest messages as and when we need to 
	for(i=0; i < mPeerList.size(); i++)
	{
		cTorrentPeer* pPeer = mPeerList[i];
		assert(pPeer);


	

		if( pPeer && pPeer->Socket().ConnectionEstablished())
		{
			if(!pPeer->HandshakeSent())
			{
				SendHandshakeMessage(*pPeer, InfoHash(),PeerId());
			}
			else
			{
				u32 tempPieceNumber;
				// Sort out if we are interested
				// NextPieceToDownload is a check to make sure that we don't already have everything they have queued.
				if(pPeer->AmInterested() == false && 
				   PeerHasPiecesWeWant(pPeer) && 
				   (NextPieceToDownload(&tempPieceNumber, pPeer) == true))
				{
					SendInterestedMessage(*pPeer);
					break;
				}
			}
		}
	}


	if(mPeerList.size() > 0)
	{
		ProcessChoking();
	}
}// END ProcessPeerConnections



bool UDgreater(cTorrentPeer* lhs, cTorrentPeer* rhs);
bool UDgreater(cTorrentPeer* lhs, cTorrentPeer* rhs)
{
	return lhs->DownloadRate() > rhs->DownloadRate();
}


// choke / unchoke peers
void cBitTorrent::ProcessChoking()
{
	// TODO : do not unchoke a peer that isn't interested

	if((mTimer.ElapsedMs() - mLastChokeProcess) >= CHOKE_PROCESS_PERIOD_MS)
	{	
		mLastChokeProcess = mTimer.ElapsedMs();

		// from the spec (i'm using backwards upload / download terminology here) ...


		// build a list of peers ordered by download rate
		std::vector<cTorrentPeer*> peersOrderedByDownloadRate;		
		for(u32 i=0; i < mPeerList.size(); i++)
		{
			peersOrderedByDownloadRate.push_back(mPeerList[i]);
		}
		std::stable_sort(peersOrderedByDownloadRate.begin(), peersOrderedByDownloadRate.end(), UDgreater);

		cTorrentPeer* peersWeWantUnchoked[MAX_PEERS_UNCHOKED];
		memset(peersWeWantUnchoked, 0, sizeof(peersWeWantUnchoked));
		u32 unchokeCount=0;

		// Add the rolling unchoke peer
		if(mRollingUnChoke != INVALID_PEER_INDEX &&
			mRollingUnChoke < mPeerList.size() &&
			mPeerList[mRollingUnChoke]->IsInterestedInMe())
		{			
			peersWeWantUnchoked[0] = mPeerList[mRollingUnChoke];
			unchokeCount++;
		}

		// unchoke the four peers which have the best download rate and are interested
		//if(NumPeersUnchoked() < MAX_PEERS_UNCHOKED)
		{
			for(u32 i=0; i < peersOrderedByDownloadRate.size(); i++)
			{
				if( unchokeCount < MAX_PEERS_UNCHOKED && 
					peersOrderedByDownloadRate[i]->HandshakeReceived() && 
					peersOrderedByDownloadRate[i]->IsInterestedInMe())
				{
					peersWeWantUnchoked[unchokeCount] = peersOrderedByDownloadRate[i];
					unchokeCount++;
				}
			}
		}


		// peers must not be interested in me, unchoke the best downloaders
		if(unchokeCount < MAX_PEERS_UNCHOKED)
		{
			for(u32 i=0; i < peersOrderedByDownloadRate.size(); i++)
			{
				if( unchokeCount < MAX_PEERS_UNCHOKED && 
					peersOrderedByDownloadRate[i]->HandshakeReceived() &&
					peersOrderedByDownloadRate[i]->IsInterestedInMe())
				{
					//SendUnChokeMessage(*peersOrderedByDownloadRate[i]);
					
					bool inList=false;
					for(u32 j=0; j < unchokeCount; j++)
					{
						if(peersOrderedByDownloadRate[i] == peersWeWantUnchoked[unchokeCount])
						{
							inList = true;
						}
					}

					if(!inList)
					{
						peersWeWantUnchoked[unchokeCount] = peersOrderedByDownloadRate[i];
						unchokeCount++;
					}
				}
			}
		}


		// someone is always unchoked
//		if(AmChokingAllPeers())
//		{
//			u32 rnd = Rand32(mPeerList.size());
//			SendUnChokeMessage(*mPeerList[rnd]);
//		}



		// we have our list of peers to unchoke, unchoke them and choke everyone else
		for(u32 i=0; i < mPeerList.size(); i++)
		{
			cTorrentPeer* pPeer = mPeerList[i];

			bool inList=false;
			for(u32 j=0; j < unchokeCount; j++)
			{				
				if(pPeer == peersWeWantUnchoked[j])
				{
					inList = true;
					break;
				}
			}

			if(inList)
			{
				if(pPeer->AmChoking())
				{
					SendUnChokeMessage(*pPeer);
				}
			}
			else
			{
				// note we do not choke the rolling unchoke peer
				if(pPeer->AmChoking()==false)
				{
					SendChokeMessage(*pPeer);
				}
			}
		}

		//Printf("BitTorrent: Currently %d peers unchoked\n", NumPeersUnchoked());
		assert(NumPeersUnchoked() <= MAX_PEERS_UNCHOKED);
	}


	// we go through all peers and they are each unchoked for ROLLING_UNCHOKE_PERIOD ms noe by one, everyone gets a go
	// all we do here is select the index, the choke algorthim above will actually unchoke the peer
	if((mTimer.ElapsedMs() - mLastRollingChokeProcess) >= ROLLING_UNCHOKE_PERIOD)
	{	
		mLastRollingChokeProcess = mTimer.ElapsedMs();
		
		if(mRollingUnChoke == INVALID_PEER_INDEX)
		{
			mRollingUnChoke = Rand32(MAX_PEER_CONNECTIONS_PER_TORRENT);
		}

		bool goodRollingUnchokeFound=false;
		for(u32 i = 0; i < MAX_PEER_CONNECTIONS_PER_TORRENT; i++)
		{
			mRollingUnChoke++;
			if(mRollingUnChoke >= mPeerList.size())
			{
				mRollingUnChoke = 0;
			}
			if(mPeerList[mRollingUnChoke] &&
			   mPeerList[mRollingUnChoke]->HandshakeReceived() &&
			   mPeerList[mRollingUnChoke]->IsInterestedInMe())
			{
				Printf("BitTorrent: Rolling unchoke is now peer %d %s\n", mRollingUnChoke, mPeerList[mRollingUnChoke]->Address().Ip().AsString());
				goodRollingUnchokeFound=true;
				break;
			}
		}

		if(!goodRollingUnchokeFound)
		{
			mRollingUnChoke = INVALID_PEER_INDEX;
		}
	}
}// END ProcessChoking



void cBitTorrent::ProcessDownloadQueue()
{
	// When we enter end game, queue all the remaining pieces
	if( IsEndGame() && 
		NumberOfPiecesDownloading() < END_GAME_THRESHHOLD &&
		NumberOfPiecesMissing() > NumberOfPiecesDownloading())
	{
		QueueAllRemainingPieces();
	}


	u32 i, j;

	// TODO : go through the mDownloadingPieces array and finish up the pieces that are closest to being done
	// the algorithm below isn't doing that, it thrashes index 0 !!!

	// get the pieces, find next block that needs downloading and an idle peer ...
	for(i=0; i < NumberOfPiecesDownloading(); i++)
	{
		assert(mDownloadingPieces[i]->IsValid());

		if(!mDownloadingPieces[i]->AllBlocksPending())
		{
			for(j=0; j < mPeerList.size(); j++)
			{
				cTorrentPeer* pPeer = mPeerList[j];

				//Printf("1 peer %X pPeer.HasOutstandingBlocksToDownload %d block %X\n", pPeer, pPeer->HasOutstandingBlocksToDownload(), pPeer->mDownloadPiece);

				if( pPeer->IsReadyToDownloadBlock() && 
					pPeer->HasPiece(mDownloadingPieces[i]->PieceNumber()) && 
					pPeer->HasBlockQueuedForDownload(mDownloadingPieces[i]->PieceNumber(), mDownloadingPieces[i]->NextMissingBlockIndex()) == false)
				{
#if DUPE_DL_CHECK
					// TODO : REDO
					/*
					// if its not end game we should never have dupe slots on the dl queue
					if(!IsEndGame())
					{
						// during 'end game' this is a good thing, we just need to cancel blocks as we 
						// get them, if its not end game this is a good check and a good assert
						for(u32 k=0; k < mPeerList.size(); k++)
						{
							if(k==j) continue;

							if( mPeerList[k]->HasAnyOustandingRequestsForPiece(mDownloadingPieces[i].PieceNumber()) &&
								mPeerList[k]->BlockNumberDownloading() == mDownloadingPieces[i].NextMissingBlockIndex())
							{
								// this piece is already downloading
								assert(0);
							}
						}
					}
					*/
#endif
					//Printf("2 peer %X pPeer.HasOutstandingBlocksToDownload %d block %X\n", pPeer, pPeer->HasOutstandingBlocksToDownload(), pPeer->mDownloadPiece);
					pPeer->RequestBlock(mDownloadingPieces[i], mDownloadingPieces[i]->NextMissingBlockIndex());
					break;			
				}
			}	
		}
	}


	// keep the download queue filled with good pieces 
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		cTorrentPeer* pPeer = mPeerList[i];

		if(PeerHasPiecesWeWant(pPeer) == false)
		{
			continue;
		}

		const cTorrentPiece* piece = PeerHasAQueuedPieceTheyCanDownload(mPeerList[i]);
		
		if( pPeer->HandshakeReceived() && 
			!pPeer->HasOutstandingBlocksToDownload() &&
			!piece)
		{
			u32 newPiece;
			bool ret = NextPieceToDownload(&newPiece, pPeer);
			//assert(ret);
			if(!ret)
			{
				if(pPeer->HandshakeReceived() && pPeer->BitfieldReceived() && PeerHasPiecesWeWant(pPeer))
				{
					//Printf("BitTorrent: Peer has pieces we need but nothing to download %s\n", mPeerList[i]->Address().Ip().AsString());
				}
				continue;
			}

			cTorrentPiece* pNewQueuedPiece = QueuePieceForDownload(newPiece);
			assert(pNewQueuedPiece);
		}
	}

}// END ProcessDownloadQueue



void cBitTorrent::ProcessUploadQueue()
{

	// TODO : THIS NEEDS TO SORT OUT THE GOOD PEERS FIRST


	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if( mPeerList[i]->Socket().ConnectionEstablished() && 
			mPeerList[i]->HandshakeReceived() &&
			mPeerList[i]->HasUploadRequestsOutstanding())
		{
			if(WantToUploadAPieceThisPeer(*mPeerList[i]))
			{
				mPeerList[i]->SendUploadRequest(this);
			}				
		}
	}
}// END ProcessUploadQueue



bool cBitTorrent::AConnectedPeerHasThisPiece(u32 pieceNumber) const
{
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if(mPeerList[i]->Socket().ConnectionEstablished() && mPeerList[i]->HandshakeReceived())
		{
			if(mPeerList[i]->HasPiece(pieceNumber))
			{
				return true;
			}
		}
	}
	return false;
}// END AConnectedPeerHasThisPiece


// Returns the rarest piece that we do not currently have
u32 cBitTorrent::RarestPiece(const cTorrentPeer* pPeerMustHave) const
{
	u32* pieceCounts = new u32[FileSet().NumberOfPieces()];
	memset(pieceCounts, 0, FileSet().NumberOfPieces() * sizeof(u32));

	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		for(u32 j=0; j < mPeerList.size(); j++)
		{
			if(mPeerList[j]->HasPiece(i))
			{
				pieceCounts[i]++;
			}
		}
	}


	u32 lowest=0xFFFFFFFF;
	u32 pieceIndex=INVALID_PIECE_NUM;;
	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		if(HavePiece(i) || IsPieceQueued(i))
		{
			continue;
		}

		if(pPeerMustHave && (pPeerMustHave->HasPiece(i) == false))
		{
			continue;
		}

		if(pieceCounts[i] < lowest)
		{
			lowest = pieceCounts[i];
			pieceIndex = i;
		}
	}

	delete[] pieceCounts;

	return pieceIndex;
}// END RarestPiece



bool cBitTorrent::NextPieceToDownload(u32* pieceOut, const cTorrentPeer* pPeerMustHave) const
{
	assert(!AllPiecesDownloaded());

	u32 pieceNum = INVALID_PIECE_NUM;

	bool isEndGame = IsEndGame();



	
	if(!isEndGame)
	{
#if 0
		///////////////////////////////////////////////////////////////////////////
		// random piece

		u32 num = Rand32(FileSet().NumberOfPieces());
		// we don't have it and we are not already downloading it
		if(mPiecesBitfield.Get(num) == 0 && !IsPieceQueued(num) && pPeerMustHave->HasPiece(num)/*AConnectedPeerHasThisPiece(num)*/)
		{
			pieceNum = num;
		}
#endif

		///////////////////////////////////////////////////////////////////////////
		// Rarest piece
		u32 num = RarestPiece(pPeerMustHave);
		if(num != INVALID_PIECE_NUM)
		{
			assert(mPiecesBitfield.Get(num) == 0 && !IsPieceQueued(num));
			if(mPiecesBitfield.Get(num) == 0 && !IsPieceQueued(num) && pPeerMustHave->HasPiece(num))
			{
				pieceNum = num;
			}
		}
	}
	
	
	if(pieceNum == INVALID_PIECE_NUM)
	{
		///////////////////////////////////////////////////////////////////////////
		// First Missing
		// First pass we look for unqueued pieces, second pass (for end game only) we allow dupes if nothing else is there
		pieceNum =  FirstMissingPiece(false, pPeerMustHave);
		if( pieceNum == INVALID_PIECE_NUM && 
			isEndGame)
		{
			pieceNum =  FirstMissingPiece(true, pPeerMustHave);
		}
		///////////////////////////////////////////////////////////////////////////
	}


	///////////////////////////////////////////////////////////////////////////
	// TODO RAREST Piece
	///////////////////////////////////////////////////////////////////////////


	*pieceOut = pieceNum;
	return pieceNum != INVALID_PIECE_NUM;
}// END NextPieceToDownload



bool cBitTorrent::IsPieceQueued(u32 pieceNumber) const
{
	for(u32 i=0; i < NumberOfPiecesDownloading(); i++)
	{
		if(mDownloadingPieces[i]->PieceNumber() == pieceNumber)
		{
			return true;
		}
	}
	return false;
}// END IsPieceQueued



bool cBitTorrent::AreAllRemainingPiecesQueued() const
{
	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		if(mPiecesBitfield.Get(i) == 0)
		{
			if(!IsPieceQueued(i))
			{
				return false;
			}
		}
	}
	return true;
}// END AreAllRemainingPiecesQueued



void cBitTorrent::ClearDownloadPiece(u32 pieceNumber)
{
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		// During endgame we will have multiple peers downloading the same piece, cancel them.
		// Lets always do this check...
		//if(IsEndGame())
		{
			if(mPeerList[i]->OutstandingRequestQueue().HasAnyOustandingRequestsForPiece(pieceNumber))
			{
				mPeerList[i]->OutstandingRequestQueue().CancelAllPieceRequests(pieceNumber);
			}
		}
	}


	u32 count=0;
	bool deleted = true;

	// If it is end game then we may have a piece queued multiple times, ensure we delete them all
	while(deleted)
	{
		deleted = false;

		// clear this download queue piece
		for(TorrentPieceVectorIterator iter = mDownloadingPieces.begin(); iter != mDownloadingPieces.end(); iter++)
		{
			if((*iter)->PieceNumber() == pieceNumber)
			{
				delete (*iter);

				mDownloadingPieces.erase(iter);

				deleted = true;
				count++;
				break;
			}
		}
	}
	// not found?!
	assert(count > 0);
}// END ClearDownloadPiece



s32 cBitTorrent::FirstMissingPiece(bool allowQueuedPieces, const cTorrentPeer* pPeerMustHave) const
{
	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		if(pPeerMustHave && pPeerMustHave->HasPiece(i)==false)
		{
			continue;
		}

		if(mPiecesBitfield.Get(i) == 0)
		{
			
			if(!IsPieceQueued(i))
			{
				return i;
			}
			else
			{
				if(allowQueuedPieces)
				{
					return i;
				}
			}
		}
	}

	// all downloaded or peer doesn't have piece
	return INVALID_PIECE_NUM;
}// END FirstMissingPiece



void cBitTorrent::OnPieceDownloaded(u32 pieceIndex)
{
	mNumberOfPiecesDownloading--;

	// thrashing?
	//FileSet().Flush();

	if(FileSet().IsLocalPieceValid(pieceIndex))
	{
		mPiecesBitfield.Set(pieceIndex);

#if LOAD_DL_STATE
		std::string fn = mMetaFileName + ".mk";
		mPiecesBitfield.WriteToDisk(fn.c_str());		
#endif

		// send all our peers the HAVE message and if its endgame then cancel dupe requests
		for(u32 i=0; i < mPeerList.size(); i++)
		{
			if(mPeerList[i]->Socket().ConnectionEstablished() && mPeerList[i]->HandshakeReceived())
			{
				SendHaveMessage(*mPeerList[i], pieceIndex);
			}
		}

		//Printf("BitTorrent: PIECE DOWNLOADED %d / %d\n", pieceIndex, FileSet().NumberOfPieces());
		//Printf("BitTorrent: Total DOWNLOADED %d / %d\n", NumberOfPiecesDownloaded(), FileSet().NumberOfPieces());
	}
	else
	{
		Printf("BitTorrent: %d CORRUPT PIECE DOWNLAODED!\n", pieceIndex);
	}


	// Clear this piece from the download queue, this will also remove it from all peers download queues.
	ClearDownloadPiece(pieceIndex);


#if DUPE_DL_CHECK
	if(IsPieceQueued(pieceIndex))
	{
		assert(0);
	}
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		// someone still downloading this completed piece?!
		if(mPeerList[i]->OutstandingRequestQueue().HasAnyOustandingRequestsForPiece(pieceIndex))
		{
			assert(0);
		}
	}
#endif // DUPE_DL_CHECK
}// END OnPieceDownloaded



void cBitTorrent::OnBlockDownloaded(u32 pieceIndex, u32 blockIndex, u32 blockSize)
{
	mBytesDownloadedInThisSession += blockSize;

	// during end game we have the same block in multiple slots, make sure the dl status is updated in all of them
	if(IsEndGame())
	{
		for(u32 i=0; i < NumberOfPiecesDownloading(); i++)
		{
			if(mDownloadingPieces[i]->PieceNumber() == pieceIndex && 
				mDownloadingPieces[i]->GetBlockStatus(blockIndex) != cTorrentPiece::BLOCK_DOWNLOADED)
			{
				mDownloadingPieces[i]->SetBlockStatus(blockIndex, cTorrentPiece::BLOCK_DOWNLOADED, 0, cSockAddr());
			}
		}
	}

	for(u32 i=0; i < NumberOfPiecesDownloading(); i++)
	{
		assert(mDownloadingPieces[i]->IsValid());

		if(mDownloadingPieces[i]->HasDownloaded())
		{
			OnPieceDownloaded(pieceIndex);
			break;
		}
	}
}// END OnBlockDownloaded



void cBitTorrent::OnBlockUploaded(u32 pieceIndex, u32 blockIndex, u32 blockSize)
{
	mBytesUploadedInThisSession += blockSize;
}// END OnBlockUploaded



bool cBitTorrent::IsEndGame() const
{
	return mIsEndGame; 
}// END IsEndGame



void cBitTorrent::CacheIsEndGame()
{
	mIsEndGame = (AreAllRemainingPiecesQueued() || (NumberOfPiecesMissing() <= END_GAME_THRESHHOLD));
}// END CacheIsEndGame



u32 cBitTorrent::NumberOfPiecesDownloaded() const
{
	// TODO : cache this at start and add to it whenever a piece downloads
	u32 count=0;
	for(u32 i=0; i < FileSet().NumberOfPieces(); i++)
	{
		if(mPiecesBitfield.Get(i) == 1)
		{
			count++;
		}
	}
	return count;
}// END NumberOfPiecesDownloaded



u32 cBitTorrent::NumberOfPiecesMissing() const
{
	return FileSet().NumberOfPieces() - NumberOfPiecesDownloaded();
}// END NumberOfPiecesMissing



bool cBitTorrent::AllPiecesDownloaded() const
{
	return NumberOfPiecesDownloaded() == FileSet().NumberOfPieces();
}// END AllPiecesDownloaded



bool cBitTorrent::WantToUploadAPieceThisPeer(const cTorrentPeer& peer)
{
	if(BitTorrentManager().UploadRate() >= BitTorrentManager().MaxUploadRate())
	{
		//Printf("BitTorrent: Upload limit reached, refuse piece request\n");
		return false; 
	}

	if(peer.HandshakeReceived() == false ||
		peer.Socket().ConnectionEstablished() == false)
	{
		return false;
	}

	if(peer.AmChoking())
	{
		// we are choking this peer, they have no right to request
		// disconnect?
		//assert(0);

		Printf("BitTorrent: Choked peer making a request %s DENIED\n", peer.Address().Ip().AsString());

		return false;
	}

	return true;
}// END WantToUploadAPieceThisPeer



u32 cBitTorrent::DownloadRate() const
{
	u32 total=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		total += mPeerList[i]->DownloadRate();
	}
	return total;
}// END DownloadRate



u32 cBitTorrent::UploadRate() const
{
	u32 total=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		total += mPeerList[i]->UploadRate();
	}
	return total;
}// END UploadRate




u32 cBitTorrent::NumPeersUnchoked() const
{
	u32 total=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if(mPeerList[i]->AmChoking() == false)
		{
			total++;
		}
	}
	return total;
}// END NumPeersUnchoked



// how long to finish downloading (in seconds)
s64 cBitTorrent::Eta() const
{
	if(DownloadRate() == 0)
	{
		return -1;
	}
	// assumes final piece size is standard length

	s64 bytesDownloaded = s64(NumberOfPiecesDownloaded()) * s64(FileSet().PieceSize());
	s64 remainingBytes = FileSet().TotalFileSize() - bytesDownloaded;
	u32 downlaodRate = DownloadRate();

	return s64(remainingBytes / downlaodRate);
}// END Eta



void cBitTorrent::ProcessState_CreateFiles()
{
	switch(FileSet().FilesCreationState())
	{
		case cFilePieceSet::FCR_CLOSED:
			ASSERT_MSG(0, "Files have not tried to open?");
			break;

		case cFilePieceSet::FCR_WORKING:
			break;

		case cFilePieceSet::FCR_FAILED:
			Printf("BitTorrent ERROR: File Creation failed\n");
			Stop();
			break;

		case cFilePieceSet::FCR_SUCCESS:
		{
			switch(mPostLoadState)
			{
			case SEED_MODE:
			case PEER_MODE:
				if(AllPiecesDownloaded())
				{
					mState = SEED_MODE;
				}
				else
				{
					mState = PEER_MODE;
				}
				break;

			case STOPPED:
				mState = STOPPED;
				break;

			case QUEUED:
				mState = QUEUED;
				break;
			}
		}
		break;
	}
}// END ProcessState_CreateFiles



void cBitTorrent::ProcessState_PeerMode()
{
	if(mState == PEER_MODE &&
	   AllPiecesDownloaded())
	{
		FileSet().Flush();

		// TODO : Get rid of these awful flags!
		SetEventFlag(BT_COMPLETE, true);

		// Fire disconnected callbacks
		for(u32 i=0; i < mDownloadCompelteHandlers.Size(); i++)
		{
			DownloadCompleteCallbackSet::sCallback sCb = mDownloadCompelteHandlers.GetCallback(i);
			sCb.mCbFn(this, sCb.mParam);
		}

		Printf("BitTorrent: Finished downloading torrent, woo\n");

		if(BitTorrentManager().Options().RecheckOnCompletion())
		{
			//Printf("BitTorrent: Finished validate\n");
			Recheck();
		}
		else if(BitTorrentManager().Options().StopOnCompletion())
		{
			Stop();
		}
		else
		{
			mState = SEED_MODE;
		}
	}
	else
	{
		if(mIpPool.size() == 0 &&
		   mPeerList.size() == 0)
		{
			// no peers yet
			return;
		}

		ProcessPeerConnections();

		ProcessReceivedMessages();

		ProcessDownloadQueue();

		ProcessUploadQueue();
	}
}// END ProcessState_Sharing



void cBitTorrent::ProcessState_SeedMode()
{
	ProcessPeerConnections();
	ProcessReceivedMessages();
	ProcessUploadQueue();
}// END ProcessState_SeedMode



void cBitTorrent::ProcessState_Rechecking()
{
	bool pieceValid = FileSet().IsLocalPieceValid(mNextPieceForRecheck);
	if(pieceValid == false)
	{
		if(mPiecesBitfield.Get(mNextPieceForRecheck) == 1)
		{
			Printf("BitTorrent: Piece %d not valid but marked valid, redownload required\n", mNextPieceForRecheck);				
		}
	}
	mPiecesBitfield.Set(mNextPieceForRecheck, pieceValid);
	
	mNextPieceForRecheck++;


	if(mNextPieceForRecheck >= FileSet().NumberOfPieces())
	{
#if LOAD_DL_STATE
		std::string fn = mMetaFileName + ".mk";
		mPiecesBitfield.WriteToDisk(fn.c_str());		
#endif

		Stop();
	}

	//Printf("Recheck result: %u / %u\n", have, numPieces);
}// END ProcessState_Rechecking


