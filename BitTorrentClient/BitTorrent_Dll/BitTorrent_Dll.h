#include <Windows.h>

#include <Network/BitTorrent/BitTorrentValues.h>
#include <Network/BitTorrent/BitTorrentCallbacks.h>
#include <Network/BitTorrent/BitTorrentManager.h>
#include <Network/BitTorrent/AnnounceManager.h>


// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the BITTORRENT_DLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// BITTORRENT_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BITTORRENT_DLL_EXPORTS
#define BITTORRENT_DLL_API __declspec(dllexport)
#else
#define BITTORRENT_DLL_API __declspec(dllimport)
#endif


// NB : Don't marshal bool, c# thinks its 1 byte, marshal BOOL instead


extern "C" __declspec (dllexport) void APIENTRY DebugDll();

//extern "C" __declspec (dllexport) void APIENTRY SetStdOutHandle(HANDLE hStdOut);
extern "C" __declspec (dllexport) void APIENTRY SetDebugStringOutputCb(DebugStringOutputCb cb);

extern "C" __declspec (dllexport) void APIENTRY InitTorrentManager(const char* appDataFolder);
extern "C" __declspec (dllexport) void APIENTRY DeInitTorrentManager();
extern "C" __declspec (dllexport) u32  APIENTRY AddTorrent(const char* fn, const char* rootDir);
extern "C" __declspec (dllexport) BOOL APIENTRY RemoveTorrent(u32 torrentId);
extern "C" __declspec (dllexport) void APIENTRY UpdateTorrentManager();
extern "C" __declspec (dllexport) cBitTorrentManager::BtmState APIENTRY BitTorrentManagerState();

extern "C" __declspec (dllexport) BOOL APIENTRY SaveTorrentManagerState();
extern "C" __declspec (dllexport) BOOL APIENTRY LoadTorrentManagerState();

extern "C" __declspec (dllexport) BOOL APIENTRY TorrentValid(u32 torrentId);
extern "C" __declspec (dllexport) void APIENTRY TorrentForceRecheck(u32 torrentId);

extern "C" __declspec (dllexport) BOOL APIENTRY IsDhtRunning();
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfDhtNodes();

extern "C" __declspec (dllexport) void APIENTRY SetListenerPort(u16 port);
extern "C" __declspec (dllexport) u16 APIENTRY GetListenerPort();

extern "C" __declspec (dllexport) void APIENTRY StartTorrent(u32 torrentId);
extern "C" __declspec (dllexport) void APIENTRY StopTorrent(u32 torrentId);
extern "C" __declspec (dllexport) void APIENTRY PauseTorrent(u32 torrentId);


extern "C" __declspec (dllexport) u32 APIENTRY TotalDownloadRate();
extern "C" __declspec (dllexport) u32 APIENTRY TotalUploadRate();

extern "C" __declspec (dllexport) u32 APIENTRY TimeTorrentManagerRunning();

extern "C" __declspec (dllexport) void APIENTRY GetTorrentClientOptions(sTorrentOptions& optionsOut);
extern "C" __declspec (dllexport) void APIENTRY SetTorrentClientOptions(sTorrentOptions options);


extern "C" __declspec (dllexport) void APIENTRY SetTorrentOnlyUsesEncryptedConnections(u32 torrentId, BOOL allow);
extern "C" __declspec (dllexport) BOOL APIENTRY DoesTorrentOnlyUsesEncryptedConnections(u32 torrentId);
extern "C" __declspec (dllexport) void APIENTRY DisconnectAllUnencryptedPeers(u32 torrentId);


typedef struct  
{
	TorrentHandle mHandles[64];
	u32  mNumHanles;
}sTorrentHandles;
extern "C" __declspec (dllexport) void APIENTRY GetAllTorrentHandles(sTorrentHandles& handleArray);


typedef struct  
{
	u32  mHandle;
	cBitTorrent::TorrentState mState;
	char mName[256];
	char mFileName[256];
	char mTargetFolder[1024];
	char mComment[512];
	u8   mInfoHash[20];
	u32	 mCreationDate;
	u32  mTotalPieces;
	u32  mPiecesDownloaded;
	u32  mPieceSize;
	s64  mTotalSize;
	u32  mTimeSinceStarted;
	s64  mEta;
	u32  mDownloadSpeed;
	u32  mUploadSpeed;
	u32  mEventFlags;
	u32  mNumEncryptedConnections;
	u32  mNumUnencryptedConnections;
	u32  mNumSeeds;
	u32  mNumPeers;
}TorrentMetaData;
extern "C" __declspec (dllexport) void APIENTRY GetTorrentMetaData(u32 torrentId, TorrentMetaData& meta);


typedef struct
{
	u8 mPeerId[PEER_ID_LENGTH];
	u32 mDlRate;
	u32 mUlRate;
	u32 mTotalBytesDownloaded;
	u32 mTotalBytesUploaded;
	u32 mOustandingDownloadRequests;
	u32 mOustandingUploadRequests;
	u32 mConnectionLengthInSeconds;

	u8 mIp[4];
	u16 mPort;

	u8 mAmChoking;
	u8 mIsChokingMe;
	u8 mAmInterested;
	u8 mIsInterestedInMe;

	float mPercentageDone;

	BOOL mHandshakeRecvd;
	BOOL mIsSeed;
	
	enum 
	{ 
		INCOMING_CONNECTION = 1 << 0,
		ENCRYPTED_CONNECTION = 1 << 1
	};
	u8 mConnectionFlags;
}sPeerInfo;
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfConnectedPeers(u32 torrentId);
extern "C" __declspec (dllexport) u32 APIENTRY GetConnectedPeersInfo(u32 torrentId, sPeerInfo** pOut, u32 maxItems);
extern "C" __declspec (dllexport) BOOL APIENTRY GetPeerMetaData(u32 torrentId, u32 peerIp, u16 peerPort, sPeerInfo& meta);



extern "C" __declspec (dllexport) u32 APIENTRY NumberOfAnnounceTargets(u32 torrentId);
extern "C" __declspec (dllexport) u32 APIENTRY GetAnnounceTargetsInfo(u32 torrentId, cAnnounceManager::AnnounceDetails** pOut, u32 maxItems);


typedef struct
{
	u32 mPieceNumber;
	u32 mPieceSize;
	u32 mNumberOfBlocks;
	u32 mCompletedBlocks;
	u32 mOutstandingBlocks;
	u32 mRequestedBlocks;
//	BOOL mIsFinalPiece;
}sPiecesInfo;
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfActivePieces(u32 torrentId);
extern "C" __declspec (dllexport) u32 APIENTRY GetActivePiecesInfo(u32 torrentId, sPiecesInfo** pOut, u32 maxItems);



typedef struct
{
	char mszFilename[512];
	s64 mSize;
	float mPercentageComplete;
	u32 mNumberOfPieces;
}sFileInfo;
extern "C" __declspec (dllexport) u32 APIENTRY NumberOfFilesInTorrent(u32 torrentId);
extern "C" __declspec (dllexport) u32 APIENTRY GetTorrentFilesInfo(u32 torrentId, sFileInfo** pOut, u32 maxItems);



// Events
extern "C" __declspec (dllexport) void APIENTRY AddTorrentAddedCallback(TorrentAddedCallback cb);
extern "C" __declspec (dllexport) void APIENTRY AddTorrentRemovedCallback(TorrentRemovedCallback cb);
extern "C" __declspec (dllexport) void APIENTRY RemoveTorrentAddedCallback(TorrentAddedCallback cb);
extern "C" __declspec (dllexport) void APIENTRY RemoveTorrentRemovedCallback(TorrentRemovedCallback cb);

extern "C" __declspec (dllexport) void APIENTRY AddPeerConnectedCallback(PeerConnectedCallback cb);
extern "C" __declspec (dllexport) void APIENTRY AddPeerDisconnectedCallback(PeerDisconnectedCallback cb);
extern "C" __declspec (dllexport) void APIENTRY RemovePeerConnectedCallback(PeerConnectedCallback cb);
extern "C" __declspec (dllexport) void APIENTRY RemovePeerDisconnectedCallback(PeerDisconnectedCallback cb);