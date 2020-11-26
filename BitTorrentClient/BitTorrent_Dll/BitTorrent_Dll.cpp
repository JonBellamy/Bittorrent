// BitTorrent_Dll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "BitTorrent_Dll.h"

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <list>
#include <vector>

#include "MeerkatLock.h"

#include <Network/BitTorrent/dht/Dht.h>
#include <Network/BitTorrent/BitTorrentPeer.h>
#include "Network/SockAddr.h"

using namespace net;


extern "C" __declspec (dllexport) void APIENTRY DebugDll()
{
	Printf("DebugDll\r\n");
}


/*
extern "C" __declspec (dllexport) void APIENTRY SetStdOutHandle(HANDLE hStdOut)
{
	//BOOL ret = SetStdHandle(STD_OUTPUT_HANDLE, hStdOut);
	int fd = _open_osfhandle( (intptr_t) hStdOut, _O_APPEND); 
	FILE* fp = _fdopen(fd, "w"); 
	*stdout = *fp; 
	setvbuf(stdout, NULL, _IONBF, 0);  
	Printf("redirecting std out\n");
}// END SetStdOutHandle
*/


extern "C" __declspec (dllexport) void APIENTRY SetDebugStringOutputCb(DebugStringOutputCb cb)
{
	cMeerkatLock lock;
	SetPrintfHandler(cb);
}// END SetDebugStringOutputCb



extern "C" __declspec (dllexport) void APIENTRY InitTorrentManager(const char* appDataFolder)
{
	cMeerkatLock::InitLock();

	cMeerkatLock lock;
	BitTorrentManager().Init(appDataFolder);
}// END InitTorrentManager



extern "C" __declspec (dllexport) void APIENTRY DeInitTorrentManager()
{
	cMeerkatLock lock;
	BitTorrentManager().DeInit();
}// END DeInitTorrentManager



extern "C" __declspec (dllexport) u32 APIENTRY AddTorrent(const char* fn, const char* rootDir)
{
	cMeerkatLock lock;
	TorrentHandle handle = BitTorrentManager().AddTorrent(fn, rootDir);
	if(handle != INVALID_TORRENT_HANDLE)
	{
		cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(handle);
		pTorrent->Start();
	}
	return handle;
}// END AddTorrent



extern "C" __declspec (dllexport) BOOL APIENTRY RemoveTorrent(u32 torrentId)
{
	cMeerkatLock lock;
	return BitTorrentManager().RemoveTorrent(torrentId);
}// END RemoveTorrent



extern "C" __declspec (dllexport) void APIENTRY UpdateTorrentManager()
{
	cMeerkatLock lock;
	BitTorrentManager().Process();
}// END UpdateTorrentManager



extern "C" __declspec (dllexport) cBitTorrentManager::BtmState APIENTRY BitTorrentManagerState()
{
	cMeerkatLock lock;
	return BitTorrentManager().State();
}// END BitTorrentManagerState



extern "C" __declspec (dllexport) BOOL APIENTRY SaveTorrentManagerState()
{
	cMeerkatLock lock;
	return BitTorrentManager().SaveState();
}// END SaveTorrentManagerState


extern "C" __declspec (dllexport) BOOL APIENTRY LoadTorrentManagerState()
{
	cMeerkatLock lock;
	return BitTorrentManager().LoadState();
}// END LoadTorrentManagerState



extern "C" __declspec (dllexport) BOOL APIENTRY TorrentValid(u32 torrentId)
{
	cMeerkatLock lock;
	return (BitTorrentManager().GetTorrent(torrentId) != NULL);
}// END TorrentValid



extern "C" __declspec (dllexport) void APIENTRY TorrentForceRecheck(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	if(pTorrent == NULL)
	{
		return;
	}
	pTorrent->Recheck();
}// END TorrentForceRecheck



extern "C" __declspec (dllexport) BOOL APIENTRY IsDhtRunning()
{
	cMeerkatLock lock;
	return BitTorrentManager().DhtTaskManager().Running();
}// END IsDhtRunning



extern "C" __declspec (dllexport) u32 APIENTRY NumberOfDhtNodes()
{
	cMeerkatLock lock;
	return BitTorrentManager().DhtTaskManager().Dht().RoutingTable().Size();
}// END NumberOfDhtNodes



extern "C" __declspec (dllexport) void APIENTRY SetListenerPort(u16 port)
{
	cMeerkatLock lock;
	BitTorrentManager().SetIncomingConnectionsListener(port);
}// END SetListenerPort



extern "C" __declspec (dllexport) u16 APIENTRY GetListenerPort()
{
	cMeerkatLock lock;
	return BitTorrentManager().ListenPort();
}// END GetListenerPort



extern "C" __declspec (dllexport) void APIENTRY StartTorrent(u32 torrentId)
{
	cMeerkatLock lock;
	BitTorrentManager().StartTorrent(torrentId);
}// END StartTorrent



extern "C" __declspec (dllexport) void APIENTRY StopTorrent(u32 torrentId)
{
	cMeerkatLock lock;
	BitTorrentManager().StopTorrent(torrentId);
}// END StopTorrent



extern "C" __declspec (dllexport) void APIENTRY PauseTorrent(u32 torrentId)
{
	cMeerkatLock lock;
	BitTorrentManager().PauseTorrent(torrentId);
}// END PauseTorrent



extern "C" __declspec (dllexport) u32 APIENTRY TotalDownloadRate()
{
	cMeerkatLock lock;
	return BitTorrentManager().DownloadRate();
}// END TotalDownloadRate



extern "C" __declspec (dllexport) u32 APIENTRY TotalUploadRate()
{
	cMeerkatLock lock;
	return BitTorrentManager().UploadRate();
}// END TotalUploadRate


extern "C" __declspec (dllexport) u32 APIENTRY TimeTorrentManagerRunning()
{
	cMeerkatLock lock;
	return BitTorrentManager().TimeTorrentManagerRunning();
}// END TimeTorrentManagerRunning


extern "C" __declspec (dllexport) void APIENTRY GetTorrentClientOptions(sTorrentOptions& optionsOut)
{
	cMeerkatLock lock;
	optionsOut = BitTorrentManager().Options();
}// END GetTorrentClientOptions



extern "C" __declspec (dllexport) void APIENTRY SetTorrentClientOptions(sTorrentOptions options)
{
	cMeerkatLock lock;
	BitTorrentManager().Options() = options;
	BitTorrentManager().OnOptionsUdated();
}



extern "C" __declspec (dllexport) void APIENTRY SetTorrentOnlyUsesEncryptedConnections(u32 torrentId, BOOL allow)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	if(pTorrent == NULL)
	{
		return;
	}
	pTorrent->ConnectionsMustBeEncrypted(allow!=0);
}// END SetTorrentOnlyUsesEncyptedConnections



extern "C" __declspec (dllexport) BOOL APIENTRY DoesTorrentOnlyUsesEncryptedConnections(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	if(pTorrent == NULL)
	{
		return false;
	}
	return pTorrent->ConnectionsMustBeEncrypted();
}// END DoesTorrentOnlyUsesEncyptedConnections



extern "C" __declspec (dllexport) void APIENTRY DisconnectAllUnencryptedPeers(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	if(pTorrent == NULL)
	{
		return;
	}
	pTorrent->DisconnectAllUnencryptedPeers();
}// END DisconnectAllUnencryptedPeers


extern "C" __declspec (dllexport) void APIENTRY GetAllTorrentHandles(sTorrentHandles& handleArray)
{
	cMeerkatLock lock;
	handleArray.mNumHanles = BitTorrentManager().GetAllTorrentHandles(&(handleArray.mHandles[0]), 64);
}// END GetAllTorrentHandles



extern "C" __declspec (dllexport) void APIENTRY GetTorrentMetaData(u32 torrentId, TorrentMetaData& meta)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);
	if(pTorrent)
	{
		meta.mHandle = torrentId;

		meta.mState = pTorrent->State();

		memset(meta.mName, 0, sizeof(meta.mName));
		strcpy_s(meta.mName, sizeof(meta.mName), pTorrent->MetaFile().Name().c_str());

		memset(meta.mFileName, 0, sizeof(meta.mFileName));
		strcpy_s(meta.mFileName, sizeof(meta.mFileName), pTorrent->FileName().c_str());

		std::string targetFolder = pTorrent->FileSet().RootFolder() + std::string("\\") + pTorrent->MetaFile().Name();
		memset(meta.mTargetFolder, 0, sizeof(meta.mTargetFolder));
		strcpy_s(meta.mTargetFolder, sizeof(meta.mTargetFolder), targetFolder.c_str());

		memset(meta.mComment, 0, sizeof(meta.mComment));
		strcpy_s(meta.mComment, sizeof(meta.mComment), pTorrent->MetaFile().Comment().c_str());

		assert(sizeof(meta.mInfoHash) == INFO_HASH_LENGTH);
		memcpy(meta.mInfoHash, pTorrent->InfoHash(), INFO_HASH_LENGTH);

		meta.mCreationDate = pTorrent->MetaFile().CreationDate();
		meta.mTotalPieces = pTorrent->FileSet().NumberOfPieces();
		meta.mPiecesDownloaded = pTorrent->NumberOfPiecesDownloaded();
		meta.mPieceSize = pTorrent->FileSet().PieceSize();
		meta.mTotalSize = pTorrent->FileSet().TotalFileSize();
		meta.mTimeSinceStarted = pTorrent->TimeSinceStarted();
		meta.mEta = pTorrent->Eta();
		meta.mDownloadSpeed = pTorrent->DownloadRate();
		meta.mUploadSpeed = pTorrent->UploadRate();
		meta.mEventFlags = pTorrent->EventFlagsAsU32();
		meta.mNumEncryptedConnections = pTorrent->NumberOfEncryptedConnections();
		meta.mNumUnencryptedConnections = pTorrent->NumberOfUnencryptedConnections();
		meta.mNumSeeds = pTorrent->NumberOfSeeds();
		meta.mNumPeers = pTorrent->NumberOfPeers();
		pTorrent->ClearAllEventFlags();
	}
}// END GetTorrentMetaData



extern "C" __declspec (dllexport) u32 APIENTRY NumberOfConnectedPeers(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);
	if(pTorrent)
	{
		return pTorrent->NumberOfPeerConnections();
	}
	return 0;
}// END NumberOfConnectedPeers



/*
extern "C" __declspec (dllexport) int ArrayTest(void** pOut)
{
	cMeerkatLock lock;
	sPeerInfo *arPtr = new sPeerInfo[10];


	for(int n = 0; n < 10; n++)
	{
		//*arPtr++ = n;
		arPtr[n].ip = 5;
		arPtr[n].port = n+1;
	}

	*pOut = arPtr;
	return 10;
}
*/


extern "C" __declspec (dllexport) u32 APIENTRY GetConnectedPeersInfo(u32 torrentId, sPeerInfo** pOut, u32 maxItems)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);

	u32 numItems=0;
	if(pTorrent)
	{
		std::vector<const cTorrentPeer*> peerList;
		pTorrent->GetAllConnectedPeers(peerList);

		numItems = min(maxItems, peerList.size());
		for(u32 i = 0; i < numItems; i++)
		{
			sPeerInfo* addr = *pOut++;

			memcpy(addr->mPeerId, peerList[i]->PeerId(), 20); 

			addr->mIp[0]  = peerList[i]->Address().Ip().GetB1();
			addr->mIp[1]  = peerList[i]->Address().Ip().GetB2();
			addr->mIp[2]  = peerList[i]->Address().Ip().GetB3();
			addr->mIp[3]  = peerList[i]->Address().Ip().GetB4();
			addr->mPort   = peerList[i]->Address().Port();
			addr->mDlRate = peerList[i]->DownloadRate();
			addr->mUlRate = peerList[i]->UploadRate();
			addr->mTotalBytesDownloaded = peerList[i]->TotalBytesDownloaded();
			addr->mTotalBytesUploaded = peerList[i]->TotalBytesUploaded();			
			addr->mOustandingDownloadRequests = peerList[i]->NumberOfOutstandingDownloadBlocks();
			addr->mOustandingUploadRequests = peerList[i]->NumberOfOutstandingUploadBlocks();			
			addr->mConnectionLengthInSeconds = (peerList[i]->ConnectionLength() / 1000);

			addr->mAmChoking = peerList[i]->AmChoking();
			addr->mIsChokingMe = peerList[i]->IsChokingMe();
			addr->mAmInterested = peerList[i]->AmInterested();
			addr->mIsInterestedInMe = peerList[i]->IsInterestedInMe();

			addr->mPercentageDone = peerList[i]->PercentageDone();

			addr->mHandshakeRecvd = peerList[i]->HandshakeReceived();
			addr->mIsSeed = peerList[i]->IsSeed();

			addr->mConnectionFlags = 0;
			if(peerList[i]->Socket().WasIncomingConnection())
			{
				addr->mConnectionFlags |= sPeerInfo::INCOMING_CONNECTION;
			}
			if(peerList[i]->Socket().IsEncryptedConnection())
			{
				addr->mConnectionFlags |= sPeerInfo::ENCRYPTED_CONNECTION;
			}
		}
	}
	return numItems;
}// END GetConnectedPeersInfo



// Get the passed peers info
extern "C" __declspec (dllexport) BOOL APIENTRY GetPeerMetaData(u32 torrentId, u32 peerIp, u16 peerPort, sPeerInfo& meta)
{
	cMeerkatLock lock;
	
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	
	if(pTorrent)
	{
		const cTorrentPeer* pPeer = pTorrent->GetPeer(cSockAddr(peerIp, peerPort));

		if(pPeer)
		{
			sPeerInfo* addr = &meta;

			memcpy(addr->mPeerId, pPeer->PeerId(), 20); 

			addr->mIp[0]  = pPeer->Address().Ip().GetB1();
			addr->mIp[1]  = pPeer->Address().Ip().GetB2();
			addr->mIp[2]  = pPeer->Address().Ip().GetB3();
			addr->mIp[3]  = pPeer->Address().Ip().GetB4();
			addr->mPort   = pPeer->Address().Port();
			addr->mDlRate = pPeer->DownloadRate();
			addr->mUlRate = pPeer->UploadRate();
			addr->mTotalBytesDownloaded = pPeer->TotalBytesDownloaded();
			addr->mTotalBytesUploaded = pPeer->TotalBytesUploaded();			
			addr->mOustandingDownloadRequests = pPeer->NumberOfOutstandingDownloadBlocks();
			addr->mOustandingUploadRequests = pPeer->NumberOfOutstandingUploadBlocks();			
			addr->mConnectionLengthInSeconds = (pPeer->ConnectionLength() / 1000);

			addr->mAmChoking = pPeer->AmChoking();
			addr->mIsChokingMe = pPeer->IsChokingMe();
			addr->mAmInterested = pPeer->AmInterested();
			addr->mIsInterestedInMe = pPeer->IsInterestedInMe();

			addr->mPercentageDone = pPeer->PercentageDone();

			addr->mHandshakeRecvd = pPeer->HandshakeReceived();
			addr->mIsSeed = pPeer->IsSeed();

			addr->mConnectionFlags = 0;
			if(pPeer->Socket().WasIncomingConnection())
			{
				addr->mConnectionFlags |= sPeerInfo::INCOMING_CONNECTION;
			}
			if(pPeer->Socket().IsEncryptedConnection())
			{
				addr->mConnectionFlags |= sPeerInfo::ENCRYPTED_CONNECTION;
			}
			return TRUE;
		}
	}
	return FALSE;
}



/*
typedef struct
{
	char mUrl[512];
	u32 mNextUpdateMs;

}sAnnounceInfo;
*/
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfAnnounceTargets(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);
	if(pTorrent)
	{
		return BitTorrentManager().AnnounceManager().NumberOfAnnounceTargets(pTorrent);
	}
	return 0;	
}// END NumberOfAnnounceTargets



extern "C" __declspec (dllexport) u32 APIENTRY GetAnnounceTargetsInfo(u32 torrentId, cAnnounceManager::AnnounceDetails** pOut, u32 maxItems)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);

	u32 numItems=0;
	if(pTorrent)
	{
		numItems = min(maxItems, BitTorrentManager().AnnounceManager().NumberOfAnnounceTargets(pTorrent));
		for(u32 i = 0; i < numItems; i++)
		{
			cAnnounceManager::AnnounceDetails* pAnnounceOut = *pOut++;
			*pAnnounceOut = BitTorrentManager().AnnounceManager().GetAnnounceDetails(pTorrent, i);
		}
	}
	return numItems;
}// END GetAnnounceTargetsInfo

/*
typedef struct
{
	u32 mPieceNumber;
	u32 mNumberOfBlocks;
	u32 mCompletedBlocks;
	u32 mOutstandingBlocks;
	u32 mRequestedBlocks;
	BOOL mIsFinalPiece;
}sPiecesInfo;
*/
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfActivePieces(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);
	if(pTorrent)
	{
		return pTorrent->NumberOfPiecesDownloading();
	}
	return 0;
}// END NumberOfActivePieces



extern "C" __declspec (dllexport) u32 APIENTRY GetActivePiecesInfo(u32 torrentId, sPiecesInfo** pOut, u32 maxItems)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);

	u32 numItems=0;
	if(pTorrent)
	{
		const cBitTorrent::TorrentPieceVector& piecesArray = pTorrent->DownloadingPiecesArray();
		numItems = min(maxItems, pTorrent->NumberOfPiecesDownloading());

		for(u32 i = 0; i < numItems; i++)
		{
			pOut[i]->mPieceNumber = piecesArray[i]->PieceNumber();
			pOut[i]->mPieceSize = piecesArray[i]->PieceSize();
			pOut[i]->mNumberOfBlocks = piecesArray[i]->NumberOfBlocks();
			pOut[i]->mCompletedBlocks = piecesArray[i]->NumberOfCompletedBlocks();			
			pOut[i]->mOutstandingBlocks = piecesArray[i]->NumberOfOutstandingBlocks();
			pOut[i]->mRequestedBlocks = piecesArray[i]->NumberOfRequestedBlocks();
		}

		return numItems;
	}
	return 0;
}// END GetActivePiecesInfo



/*
typedef struct
{
	char mszFilename[512];
	s64 mSize;
	u32 mPercentageComplete;
	u32 mNumberOfPieces;
}sFileInfo;
*/
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfFilesInTorrent(u32 torrentId)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);
	if(pTorrent)
	{
		return pTorrent->FileSet().NumberOfFiles();
	}
	return 0;
}// END NumberOfFilesInTorrent



extern "C" __declspec (dllexport) u32 APIENTRY GetTorrentFilesInfo(u32 torrentId, sFileInfo** pOut, u32 maxItems)
{
	cMeerkatLock lock;
	cBitTorrent* pTorrent = BitTorrentManager().GetTorrent(torrentId);
	assert(pTorrent);

	u32 numItems=0;
	if(pTorrent)
	{
		numItems = min(maxItems, pTorrent->FileSet().NumberOfFiles());

		for(u32 fileIndex = 0; fileIndex < numItems; fileIndex++)
		{
			strcpy_s(pOut[fileIndex]->mszFilename, sizeof(pOut[fileIndex]->mszFilename), pTorrent->FileSet().FileName(fileIndex).c_str());
			pOut[fileIndex]->mSize = pTorrent->FileSet().FileSize(fileIndex);			
			
			u32 fileNumPieces = pTorrent->FileSet().NumPiecesFileIsSpreadOver(fileIndex);
			pOut[fileIndex]->mNumberOfPieces = fileNumPieces;

			// % complete			
			u32 numHave=0;
			for(u32 i=0; i < fileNumPieces; i++)
			{
				u32 pieceNum = pTorrent->FileSet().FileStartPiece(fileIndex) + i;
				if(pTorrent->HavePiece(pieceNum))
				{
					numHave++;
				}
			}
			float onePercent = (float)(fileNumPieces) / 100.0f;
			float percentage = (float)(numHave) / onePercent;
			pOut[fileIndex]->mPercentageComplete = percentage;
		}

		return numItems;
	}
	return 0;
}// END GetActivePiecesInfo




//////////////////////////////////////////////////////////////////////////
// Events


extern "C" __declspec (dllexport) void APIENTRY AddTorrentAddedCallback(TorrentAddedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().AddTorrentAddedCallback(cb, NULL);
}


extern "C" __declspec (dllexport) void APIENTRY AddTorrentRemovedCallback(TorrentRemovedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().AddTorrentRemovedCallback(cb, NULL);
}


extern "C" __declspec (dllexport) void APIENTRY RemoveTorrentAddedCallback(TorrentAddedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().RemoveTorrentAddedCallback(cb, NULL);	
}


extern "C" __declspec (dllexport) void APIENTRY RemoveTorrentRemovedCallback(TorrentRemovedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().RemoveTorrentRemovedCallback(cb, NULL);
}


extern "C" __declspec (dllexport) void APIENTRY AddPeerConnectedCallback(PeerConnectedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().AddPeerConnectedCallback(cb, NULL);
}


extern "C" __declspec (dllexport) void APIENTRY AddPeerDisconnectedCallback(PeerDisconnectedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().AddPeerDisconnectedCallback(cb, NULL);
}


extern "C" __declspec (dllexport) void APIENTRY RemovePeerConnectedCallback(PeerConnectedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().RemovePeerConnectedCallback(cb, NULL);
}


extern "C" __declspec (dllexport) void APIENTRY RemovePeerDisconnectedCallback(PeerDisconnectedCallback cb)
{
	cMeerkatLock lock;
	BitTorrentManager().RemovePeerDisconnectedCallback(cb, NULL);
}