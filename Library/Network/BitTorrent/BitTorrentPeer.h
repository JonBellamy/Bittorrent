// Jon Bellamy 21/02/2009


#ifndef BITTORRENT_PEER_H
#define BITTORRENT_PEER_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <vector>

#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/BitTorrent/Piece.h"
#include "Network/BitTorrent/RequestQueue.h"
#include "Network/BitTorrent/TorrentPacketLog.h"
#include "Network/SockAddr.h"
#include "Network/TcpSocket.h"
#include "Network/BitTorrent/Mse/MessageStreamEncryption.h"
#include "General/Timer.h"
#include "Containers/Bitfield.h"
#include "Math/RollingAverage.h"

class cBitTorrent;
class cTorrentHandshakeMessage;


#define PEER_LOG 0


class cTorrentPeer
{
public:
	cTorrentPeer(const net::cSockAddr& addr, u32 numberOfPieces, const cTimer& torrentTimer, cBitTorrent& parentTorrent);
	virtual ~cTorrentPeer();

	void Process();

#if USE_MSE
	cEncryptedTorrentConnection& Socket() { return mSocket; }	
	const cEncryptedTorrentConnection& Socket() const { return mSocket; }
#else
	net::cTcpSocket& Socket() { return mSocket; }	
	const net::cTcpSocket& Socket() const { return mSocket; }
#endif


	void SetConnection(const cEncryptedTorrentConnection& s, const net::cSockAddr& addr);

	const net::cSockAddr& Address() const { return mAddr; }

	bool OpenConnection();
	u32 ConnectionOpenTime() const;
	bool IsPeerUsable() const;
	void CheckIsSeed();
	float PercentageDone() const;

	bool AmChoking() const { return mAmChoking; }
	void AmChoking(bool b);
	bool IsChokingMe() const { return mIsChokingMe; }
	void IsChokingMe(bool b);

	bool AmInterested() const { return mAmInterested; }
	void AmInterested(bool b);
	bool IsInterestedInMe() const { return mIsInterestedInMe; }
	void IsInterestedInMe(bool b) { mIsInterestedInMe = b; }

	bool IsSeed() const { return mIsSeed; }	

	void OnHandshakeReceived(const cTorrentHandshakeMessage& msg);
	bool HandshakeReceived() const { return mHandshakeReceived; }

	bool HandshakeSent() const { return mHandshakeSent; }
	void HandshakeSent(bool b) { mHandshakeSent = b; }

	void OnBitfieldReceived(const cBitField& bf);
	bool BitfieldReceived() const { return mBitfieldReceived; }

	cBitField& Bitfield() { return mPiecesBitField; }
	void SetBitfield(const cBitField& newBits) { mPiecesBitField = newBits; }

	bool HasPiece(u32 piece) const { return mPiecesBitField.Get(piece) != 0; }

	cBitTorrent* Torrent() { return &mParentTorrent; }

	////////////////////////////////////////////////////////////////////
	// events 

	// called whenever any message is sent/received
	void OnMessageSent();
	void OnMessageReceived();

	// called when we receive a HAVE message from the peer indicating that they have just downloaded this piece
	void OnHavePiece(u32 piece);

	void OnBlockDownloaded(cBitTorrent* pTorrent, u32 pieceIndex, u32 begin, u32 size, u32 blockNumber);
	void OnBlockRequested(cBitTorrent* pTorrent, u32 pieceIndex, u32 begin, u32 size);
	void OnBlockUploaded(cBitTorrent* pTorrent, u32 pieceIndex, u32 begin, u32 size, u32 blockNumber);



	////////////////////////////////////////////////////////////////////
	// Block Downloading

	bool RequestBlock(cTorrentPiece* pPiece, u32 blockIndex);
	void CancelOutstandingRequest(u32 pieceNumber, u32 blockIndex);
	//bool CancelledLastBlock() const { return mCancelledLastBlock; }


	bool IsReadyToDownloadBlock() const;

	bool HasBlockQueuedForDownload(u32 pieceNumber, u32 blockNumber) { return OutstandingRequestQueue().GetBlockRequest(pieceNumber, blockNumber) != NULL; }
	u32 NumberOfOutstandingDownloadBlocks() const { return mDownloadRequestQueue.Size(); }
	bool HasOutstandingBlocksToDownload() const { return NumberOfOutstandingDownloadBlocks() > 0; }
	u32 NumberOfOutstandingUploadBlocks() const { return mUploadRequestQueue.Size(); }
	bool HasOutstandingBlocksToUpload() const { return NumberOfOutstandingUploadBlocks() > 0; }

	u32 LifetimeBytesDownloaded() const { return mLifetimeBytesDownloaded; }
	u32 DownloadRate() const { return mDownloadRate.Average(); }
	u32 UploadRate() const { return mUploadRate.Average(); }

	bool SendUploadRequest(cBitTorrent* pTorrent);
	bool HasUploadRequestsOutstanding() const { return mUploadRequestQueue.Size() > 0; }

	const cRequestQueue& OutstandingRequestQueue() const { return mDownloadRequestQueue; }
	cRequestQueue& OutstandingRequestQueue() { return mDownloadRequestQueue; }

	const u8* PeerId() const { return mPeerId; }

	// How long have we been connected to this peer
	u32 ConnectionLength() const { return mTimer.ElapsedMs() - mConnectionOpenTime; }

	u32 TotalBytesDownloaded() const { return mLifetimeBytesDownloaded; }
	u32 TotalBytesUploaded() const { return mLifetimeBytesUploaded; }

	const net::cSockAddr& Addr() const { return mAddr; }

	enum
	{
		INVALID_BLOCK_NUMBER = 0xFFFFFFFF,
		CONNECTION_TIMEOUT_MS = 60000,				// Seconds for tcp connect

		KEEP_ALIVE_PERIOD = 60000,					// if no contact for minute, send a keep alive (disconnects kick in after 2 mins)

		REQUEST_TIMEOUT = 60000 * 10,				// give it x minutes for any PIECE to arrive, then sack off the peer

		MAX_OUTSTANDING_REQUESTS = 96,

		NOT_INTERESTED_DISCONNECT_PERIOD = 60000 * 5
	};


#if PEER_LOG
	cTorrentPacketLog& PacketLog() const { return mPacketLog; }
	void LogStandardMessage(u32 time, const void* pMessage, cTorrentPacketLog::MessageType messageType, HtmlPacketLogPType packetType) const { PacketLog().LogStandardMessage(time, pMessage, messageType, packetType); }
	void LogNote(u32 time, const char* strNote) const { PacketLog().LogNote(time, strNote); }
#else
	void LogStandardMessage(u32 time, const void* pMessage, cTorrentPacketLog::MessageType messageType, HtmlPacketLogPType packetType) const {}
	void LogNote(u32 time, const char* strNote) const {}
#endif

private:

	net::cSockAddr mAddr;
	const cTimer& mTimer;
	cBitTorrent& mParentTorrent;

	u32 mConnectionOpenTime;
	s32 mTcpConnectionEstablishedTime;

	bool mIsSeed;

	bool mAmChoking;
	bool mIsChokingMe;
	bool mAmInterested;
	bool mIsInterestedInMe;

	u32 mPeerChokedUsTime;
	u32 mLastTimeWeWereInterestedInPeer;

	u8 mPeerId[PEER_ID_LENGTH];

	cBitField mPiecesBitField;

	bool mHandshakeReceived;
	bool mBitfieldReceived;
	bool mHandshakeSent;

#if USE_MSE	
	cEncryptedTorrentConnection mSocket;
#else
	net::cTcpSocket mSocket;
#endif

	u32 mPeerTimeoutEnd;

	cRequestQueue mDownloadRequestQueue;		// requests we have made to others for pieces
	cRequestQueue mUploadRequestQueue;			// requests being made to us for pieces

	u32 mLastMessageSent;
	u32 mLastMessageReceived;
	u32 mLastPieceRequestTime;
	u32 mLastPieceArriveTime;


	u32 mLifetimeBytesDownloaded;
	u32 mLifetimeBytesUploaded;

	//u32 mConcurrentUnfulfilledRequests;	

	cRollingAverage<u32, 30> mDownloadRate;
	u32 mBytesDownloadedInLastSecond;

	cRollingAverage<u32, 30> mUploadRate;
	u32 mBytesUploadedInLastSecond;

	u32 mLastBandwidthRecordTime;

#if PEER_LOG
	mutable cTorrentPacketLog mPacketLog;
#endif
};



// peer meta data is kept even after connections are broken, it builds a complete history of the session
class cPeerMetaData
{
public:

	cPeerMetaData(const net::cSockAddr& addr)
	: mAddr(addr)
	, mBytesDownloaded(0)
	, mFailedConnectionAttempts(0)
	{
	}

	
	const cPeerMetaData& operator=(const cPeerMetaData& rhs)
	{
		mAddr = rhs.mAddr;
		mBytesDownloaded = rhs.mBytesDownloaded;
		mFailedConnectionAttempts = rhs.mFailedConnectionAttempts;
		return *this;
	}

	bool operator<(const cPeerMetaData& rhs)
	{
		return mFailedConnectionAttempts < rhs.mFailedConnectionAttempts;
		/*
		if(mBytesDownloaded == 0 && rhs.mBytesDownloaded == 0)
		{
			return mFailedConnectionAttempts < rhs.mFailedConnectionAttempts;
		}
		else
		{
			// one of them has downloaded something at some point
			return mBytesDownloaded > rhs.mBytesDownloaded;
		}
		*/
	}

	void OnFailedConnectionAttempt() { mFailedConnectionAttempts++; }
	void OnBytesDownloaded(u32 bytes) { mBytesDownloaded += bytes; }

	const net::cSockAddr& Addr() const { return mAddr; }
	u32 FailedConnectionAttempts() const { return mFailedConnectionAttempts; }
	

private:
	net::cSockAddr mAddr;
	u32 mBytesDownloaded;
	u32 mFailedConnectionAttempts;
};


inline u32 cTorrentPeer::ConnectionOpenTime() const 
{ 
	return mConnectionOpenTime; 
}// END ConnectionOpenTime


#endif // BITTORRENT_PEER_H
