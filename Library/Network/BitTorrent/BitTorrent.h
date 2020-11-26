// Jon Bellamy 20/02/2009


#ifndef BITTORRENT_H
#define BITTORRENT_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <vector>
#include <list>
#include <string>

#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/BitTorrent/BitTorrentCallbacks.h"
#include "Network/BitTorrent/BitTorrentMetaFile.h"
#include "Network/BitTorrent/BitTorrentPeer.h"
#include "Network/BitTorrent/FilePieceSet.h"
#include "BEncoding/BencodedDictionary.h"
#include "Network/http/HttpClient.h"
#include "Network/SockAddr.h"
#include "Network/TcpSocket.h"
#include "General/Timer.h"
#include "General/CallbackSet.h"



class cBitTorrent
{
public:
    cBitTorrent(const char* torrentMetaFile, const char* rootDir);
	~cBitTorrent();

private:
	cBitTorrent(const cBitTorrent&);
	const cBitTorrent& operator= (const cBitTorrent& rhs);


public:
	bool Parse();

	void Process();

 	bool Start();
	void Stop();
	void Pause();

	// Validate all pieces
	void Recheck();

	//void DisconnectFromPeer(cTorrentPeer* pPeer);
	cPeerMetaData* PeerMetaData(const net::cSockAddr& addr);
	const cTorrentPeer* GetPeer(const net::cSockAddr& addr) const;
	bool AmConnectedToPeer(const net::cSockAddr& sockAddr) const;	

	const cBitTorrentMetaFile& MetaFile() const;
	
	void OnPieceDownloaded(u32 pieceIndex);
	void OnBlockDownloaded(u32 pieceIndex, u32 blockIndex, u32 blockSize);
	void OnBlockUploaded(u32 pieceIndex, u32 blockIndex, u32 blockSize);

	bool AllPiecesDownloaded() const;

	u32 NumberOfPiecesDownloaded() const;
	u32 NumberOfPiecesMissing() const;

	bool WantToUploadAPieceThisPeer(const cTorrentPeer& peer);

	bool IsEndGame() const;

	// The announce manager or DHT has found some peers, add them (look for dupes)
	void PresentPeers(const std::vector<cPeerMetaData>& peerList);

	// When the torrent manager gets a new incoming peer connection, it sends it
	// to the torrent through here.
	void InsertNewConnection(cEncryptedTorrentConnection* pSocket);

	void DisconnectAllUnencryptedPeers();

	void InsertPartiallyDownloadedPiece(u32 pieceNumber, const std::string& base64Bitfield);

	u32 Time() const { return mTimer.ElapsedMs(); }

	u32 DownloadRate() const;
	u32 UploadRate() const;

	u32 BytesDownloadedInThisSession() const;
	u32 BytesUploadedInThisSession() const;

	bool AmChokingAllPeers() const;
	u32 NumPeersUnchoked() const;
	u32 NumberOfPeerConnections() const;

	// how long to finish downloading (in seconds)
	s64 Eta() const;

	std::string FileName() const { return mMetaFileName; }
	u32 TimeSinceStarted() const { return (Time() - mTimeStarted) / 1000; }

	typedef enum
	{
		STOPPED=0,
		CREATE_FILES,
		PEER_MODE,
		SEED_MODE,
		QUEUED,
		RECHECKING
	}TorrentState;

	TorrentState State() const;
	void PostLoadState(TorrentState s) { mPostLoadState = s; }
	TorrentState PostLoadState() { return mPostLoadState; }


	// mainly used for passing data back to frontend app
	void GetAllConnectedPeers(std::vector<const cTorrentPeer*>& listOut) const;



	// Vector defines
	typedef std::vector<cTorrentPiece*> TorrentPieceVector;
	typedef TorrentPieceVector::iterator TorrentPieceVectorIterator;
	typedef TorrentPieceVector::const_iterator TorrentPieceVectorConstIterator;


private:


	enum
	{
		INVALID_PIECE_NUM = 0xFFFFFFFF,		

		INVALID_PEER_INDEX = -1,

		END_GAME_THRESHHOLD = 8,					// End game starts when we are down to this many pieces outstanding

		CHOKE_PROCESS_PERIOD_MS = 10000,			// how often we choke / unchoke peers, used to avoid fibrillation
		MAX_PEERS_UNCHOKED = 5,
		ROLLING_UNCHOKE_PERIOD = 60000 * 2,			// used to ensure all peers get a unchoked fore a while
	};

	enum
	{
		RECV_BUFFER_SIZE = 1024*128
	};

	void PopulateAnnounceManager();


	////////////////////////////////////////////////////////////////////
	// peer processing

	void ProcessReceivedMessages();
	void ProcessPeerConnections();
	void ProcessChoking();

	bool ConnectToPeer(const cPeerMetaData& peer);
	void DisconnectFromPeer(u32 peerPoolIndex);
	cPeerMetaData* NextPeerToConnectTo();
	void ClearPeerList();	
	//void ProcessIncomingPeerConnections();
	const cTorrentPiece* PeerHasAQueuedPieceTheyCanDownload(const cTorrentPeer* pPeer) const;
	bool PeerHasPiecesWeWant(const cTorrentPeer* pPeer) const;


	////////////////////////////////////////////////////////////////////
	// piece downloading 

	void ProcessDownloadQueue();
	void ProcessUploadQueue();
	bool AConnectedPeerHasThisPiece(u32 pieceNumber) const;
	u32 RarestPiece(const cTorrentPeer* pPeerMustHave) const;
	bool NextPieceToDownload(u32* pieceOut, const cTorrentPeer* pPeer) const;
	s32 FirstMissingPiece(bool allowQueuedPieces, const cTorrentPeer* pPeerMustHave) const;
	bool IsPieceQueued(u32 pieceNumber) const;
	bool AreAllRemainingPiecesQueued() const;
	void ClearDownloadPiece(u32 pieceNumber);
	cTorrentPiece* QueuePieceForDownload(u32 pieceNum);	
	void QueueAllRemainingPieces();


	////////////////////////////////////////////////////////////////////
	// states 

	void ProcessState_CreateFiles();
	void ProcessState_PeerMode();
	void ProcessState_SeedMode();
	void ProcessState_Rechecking();

	void CacheIsEndGame();

public:

	////////////////////////////////////////////////////////////////////
	// inlines

	const u8* InfoHash() const;
	std::string InfoHashAsString() const;
	const u8* PeerId() const;
	std::string PeerIdAsString() const;
	const cBitField& PiecesBitfield() const;
	cFilePieceSet& FileSet();
	const cFilePieceSet& FileSet() const;
	bool ReadyForIncomingConnections() const;

	void SetEventFlag(BitTorrentEventFlag flag, bool bValue);
	bool IsEventFlagSet(BitTorrentEventFlag flag);	
	u32 EventFlagsAsU32() const;
	// this needs to be called every time the flags are read in order to clear the event
	void ClearAllEventFlags();

	bool HavePiece(u32 pieceIndex) const;

	u32 NumberOfEncryptedConnections() const;
	u32 NumberOfUnencryptedConnections() const;
	
	// How many tcp sockets are handshaking (until vista sp2 there is a hard limit of 10 half open tcp connections at once)
	u32 NumberOfHalfOpenConnections() const;

	// Note that these are reffering to peer types (seed or peer) not the number of p2p connections
	u32 NumberOfSeeds() const;
	u32 NumberOfPeers() const;
	

	bool ConnectionsMustBeEncrypted() const;
	void ConnectionsMustBeEncrypted(bool b);

	const TorrentPieceVector& DownloadingPiecesArray() const { return mDownloadingPieces; }
	u32 NumberOfPiecesDownloading() const { return mDownloadingPieces.size(); }
	
	net::cHttpClient& HttpClient() { return mHttpClient; }
	const net::cHttpClient& HttpClient() const { return mHttpClient; }	

	void AddDownloadCompleteCallback(TorrentDownloadCompleteCallback cb, void* param) { mDownloadCompelteHandlers.Add(NULL, cb, param); }
	void RemoveDownloadCompleteCallback(TorrentDownloadCompleteCallback cb, void* param) { mDownloadCompelteHandlers.Remove(NULL, cb, param); }

private:

	////////////////////////////////////////////////////////////////////
	// member data

	TorrentState mState;
	// What state should we transition to after we load
	TorrentState mPostLoadState;

	cTimer mTimer;
	u32 mTimeStarted;

	// This acts as an exemption from the global Torrent Manager rule for encrypted connections
	bool mConnectionsMustBeEncrypted;

	std::string mMetaFileName;

	cBitTorrentMetaFile mMetaFile;

	// Used for tracker announces
	net::cHttpClient mHttpClient;

	bool mIsEndGame;

	typedef std::list<cPeerMetaData>::const_iterator IpConstListIterator;
	typedef std::list<cPeerMetaData>::iterator IpListIterator;
	std::list<cPeerMetaData> mIpPool;

	std::vector<cTorrentPeer*> mPeerList;

	cFilePieceSet mFileSet;
	cBitField mPiecesBitfield;

	TorrentPieceVector mDownloadingPieces;

	u32 mNextPieceToDownload;
	u32 mNumberOfPiecesDownloading;

	u8 mInfoHash[INFO_HASH_LENGTH];

	u8 mRecvBuffer[RECV_BUFFER_SIZE];

	u8 mPeerId[PEER_ID_LENGTH];

	u32 mLastChokeProcess;

	u32 mLastRollingChokeProcess;
	u32 mRollingUnChoke;				// used to give everyone a chance to be unchoked

	u32 mEventFlags;

	u32 mNextPieceForRecheck;

	u32 mBytesDownloadedInThisSession;
	u32 mBytesUploadedInThisSession;


	// Callback handlers
	typedef cCallbackSet<NullFilterSig, TorrentDownloadCompleteCallback> DownloadCompleteCallbackSet;
	DownloadCompleteCallbackSet mDownloadCompelteHandlers;
};





//////////////////////////////////////////////////////////////////////////
// inlines


inline const cBitTorrentMetaFile& cBitTorrent::MetaFile() const
{
	return mMetaFile;
}// END MetaFile



inline const u8* cBitTorrent::InfoHash() const
{
	return mInfoHash;
}// END InfoHash



inline std::string cBitTorrent::InfoHashAsString() const
{
	std::string strHash;
	strHash.insert(0, (char*)(&mInfoHash[0]), SHA_DIGEST_LENGTH);
	return strHash;
}// END InfoHashAsString



inline const u8* cBitTorrent::PeerId() const
{
	return mPeerId;
}// END PeerId



inline std::string cBitTorrent::PeerIdAsString() const
{
	std::string peerId;
	peerId.insert(0, (char*)(&mPeerId[0]), PEER_ID_LENGTH);
	return peerId;
}// END PeerIdAsString



inline cBitTorrent::TorrentState cBitTorrent::State() const
{
	return mState;
}// END State



inline const cBitField& cBitTorrent::PiecesBitfield() const
{
	return mPiecesBitfield;
}// PiecesBitfield 



inline cFilePieceSet& cBitTorrent::FileSet() 
{ 
	return mFileSet; 
}// END FileSet



inline const cFilePieceSet& cBitTorrent::FileSet()  const
{ 
	return mFileSet; 
}// END FileSet



inline bool cBitTorrent::AmChokingAllPeers() const
{
	return (NumPeersUnchoked() == 0);
}// END AmChokingAllPeers



inline bool cBitTorrent::ReadyForIncomingConnections() const 
{
	return mState == PEER_MODE; 
}// END ReadyForIncomingConnections



inline void cBitTorrent::SetEventFlag(BitTorrentEventFlag flag, bool bValue)
{
	if(bValue)
	{
		mEventFlags |= static_cast<u32>(flag);
	}
	else
	{
		mEventFlags &= ~(static_cast<u32>(flag));
	}
}// END SetEventFlag



inline bool cBitTorrent::IsEventFlagSet(BitTorrentEventFlag flag)
{
	return (mEventFlags & static_cast<u32>(flag)) != 0;
}// END IsEventFlagSet



inline u32 cBitTorrent::EventFlagsAsU32() const
{
	return mEventFlags;
}// END EventFlagsAsU32



inline void cBitTorrent::ClearAllEventFlags()
{
	mEventFlags = 0;
}// END ClearAllEventFlags



inline bool cBitTorrent::HavePiece(u32 pieceIndex) const
{
	return mPiecesBitfield.Get(pieceIndex) != 0;
}// END HavePiece



inline u32 cBitTorrent::NumberOfEncryptedConnections() const
{
	u32 count=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if(mPeerList[i]->Socket().IsEncryptedConnection())
		{
			count++;
		}
	}
	return count;
}// END NumberOfEncryptedConnections



inline u32 cBitTorrent::NumberOfUnencryptedConnections() const
{
	u32 count=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if( mPeerList[i]->Socket().ConnectionEstablished() &&
			mPeerList[i]->Socket().IsEncryptedConnection() == false)
		{
			count++;
		}
	}
	return count;
}// END NumberOfUnencryptedConnections



inline u32 cBitTorrent::NumberOfHalfOpenConnections() const
{
	u32 count=0;

	if(HttpClient().IsConnecting())
	{
		count++;
	}
	
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if( mPeerList[i]->Socket().IsOpen() &&
			mPeerList[i]->HandshakeReceived() == false &&
			mPeerList[i]->Socket().ConnectionEstablished() == false)
		{
			count++;
		}
	}
	return count;
}// END NumberOfHalfOpenConnections



inline u32 cBitTorrent::NumberOfSeeds() const
{
	u32 count=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if( mPeerList[i]->Socket().ConnectionEstablished() && 
			mPeerList[i]->HandshakeReceived() && 
		    mPeerList[i]->IsSeed())
		{
			count++;
		}
	}
	return count;
}// END NumberOfSeeds



inline u32 cBitTorrent::NumberOfPeers() const
{
	u32 count=0;
	for(u32 i=0; i < mPeerList.size(); i++)
	{
		if( mPeerList[i]->Socket().ConnectionEstablished() && 
			mPeerList[i]->HandshakeReceived() && 
			mPeerList[i]->IsSeed() == false)
		{
			count++;
		}
	}
	return count;
}// END NumberOfPeers



inline u32 cBitTorrent::BytesDownloadedInThisSession() const 
{ 	
	return mBytesDownloadedInThisSession; 
}// END BytesDownloadedInThisSession



inline u32 cBitTorrent::BytesUploadedInThisSession() const 
{ 
	return mBytesUploadedInThisSession; 
}// END NumberOfPeersBytesUploadedInThisSession



inline bool cBitTorrent::ConnectionsMustBeEncrypted() const 
{ 
	return mConnectionsMustBeEncrypted; 
}// END ConnectionsMustBeEncrypted



inline void cBitTorrent::ConnectionsMustBeEncrypted(bool b) 
{ 
	mConnectionsMustBeEncrypted = b; 
	if(mConnectionsMustBeEncrypted)
	{
		DisconnectAllUnencryptedPeers();
	}
}// END ConnectionsMustBeEncrypted



#endif // BITTORRENT_H
