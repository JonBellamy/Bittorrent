// Jon Bellamy 21/02/2009


#include "BitTorrentPeer.h"

#include <memory.h>
#include <assert.h>

#include "Network/BitTorrent/BitTorrentMessages.h"
#include "Network/BitTorrent/BitTorrent.h"
#include "Network/BitTorrent/BitTorrentManager.h"
#include "General/Rand.h"

using namespace net;


cTorrentPeer::cTorrentPeer(const cSockAddr& addr, u32 numberOfPieces, const cTimer& torrentTimer, cBitTorrent& parentTorrent)
#if USE_MSE
: mSocket(parentTorrent.InfoHash(), SOCKET_SEND_BUFFER_SIZE, SOCKET_RECV_BUFFER_SIZE)
#else
: mSocket(INVALID_SOCKET, SOCKET_SEND_BUFFER_SIZE, SOCKET_RECV_BUFFER_SIZE)
#endif
, mAddr(addr)
, mTimer(torrentTimer)
, mParentTorrent(parentTorrent)
, mIsSeed(false)
, mAmChoking(true)
, mIsChokingMe(true)
, mPeerChokedUsTime(0)
, mLastTimeWeWereInterestedInPeer(0)
, mAmInterested(false)
, mIsInterestedInMe(false)
, mPiecesBitField(numberOfPieces, 1)
, mHandshakeReceived(false)
, mBitfieldReceived(false)
, mHandshakeSent(false)
, mLifetimeBytesDownloaded(0)
, mLifetimeBytesUploaded(0)
, mConnectionOpenTime(0)
, mTcpConnectionEstablishedTime(-1)
, mPeerTimeoutEnd(0)
, mBytesDownloadedInLastSecond(0)
, mBytesUploadedInLastSecond(0)
, mLastBandwidthRecordTime(0)
, mLastMessageSent(0)
, mLastMessageReceived(0)
, mLastPieceRequestTime(0)
, mLastPieceArriveTime(0)
{
	memset(mPeerId, 0, PEER_ID_LENGTH);

	mDownloadRequestQueue.SetPeer(this);

#if PEER_LOG
	mPacketLog.Init(BitTorrentManager().PeerLogFolder());
	std::string str = mAddr.Ip().AsString();
	str += ".html";	
	mPacketLog.Open(str.c_str());
#endif
}// END cTorrentPeer



cTorrentPeer::~cTorrentPeer()
{
	memset(mPeerId, 0xDD, PEER_ID_LENGTH);
}// END ~cTorrentPeer



void cTorrentPeer::Process()
{
#if USE_MSE
	mSocket.Process();
#endif

	if((mTimer.ElapsedMs() - mLastBandwidthRecordTime) >= 1000)
	{
		mLastBandwidthRecordTime = mTimer.ElapsedMs();
		mDownloadRate.Submit(mBytesDownloadedInLastSecond);
		mUploadRate.Submit(mBytesUploadedInLastSecond);
		mBytesDownloadedInLastSecond = 0;
		mBytesUploadedInLastSecond = 0;
	}

	if((HandshakeReceived() && 
		(mTimer.ElapsedMs() - mLastMessageSent) >= KEEP_ALIVE_PERIOD))
	{
		SendKeepAlive(*this);
	}

	// Record the time the tcp connection became established.
	if(mTcpConnectionEstablishedTime == -1 &&
		mSocket.ConnectionEstablished())
	{
		mTcpConnectionEstablishedTime = mTimer.ElapsedMs();
	}

/*
	if((mTimer.ElapsedMs() - mLastMessageReceived) >= 60 * 1000 * 2)
	{
		//Printf("DEAD peer - should disconnect\n");
		// TODO ...
	}
*/
}// END Process



void cTorrentPeer::SetConnection(const cEncryptedTorrentConnection& s, const net::cSockAddr& addr)
{
#if USE_MSE
	mSocket = s;
	mSocket.SetDontClose(false);
	mAddr = addr;
	mConnectionOpenTime = mTimer.ElapsedMs();
#else
	mSocket = s.GetSocket();
	mSocket.SetDontClose(false);
	mAddr = addr;
	mConnectionOpenTime = mTimer.ElapsedMs();
#endif
}// END SetSocket



bool cTorrentPeer::OpenConnection()
{
	mConnectionOpenTime = mTimer.ElapsedMs();
	LogNote(0, mParentTorrent.FileName().c_str());
	LogStandardMessage(0, &mAddr, cTorrentPacketLog::OUTGOING_CONNECTION, PTYPE_SEND);
	return Socket().OpenAndConnect(mAddr);	
}// END OpenConnection



bool cTorrentPeer::IsPeerUsable() const
{   
	if( mSocket.IsOpen() == false)
	{
		return false;
	}

	// Timeout tcp connection handshake (we need the handshake check here to ensure that we are not just removing closed old connections)
	if( mSocket.IsOpen() && 
		HandshakeReceived() == false &&
		mSocket.ConnectionEstablished() == false && 
		(mTimer.ElapsedMs() - mConnectionOpenTime) >= CONNECTION_TIMEOUT_MS)
	{
		LogNote(mTimer.ElapsedMs(), "Initial connection timed out\n");
		return false;
	}

	// Timeout the handshake, NB: This timer should is from the time the connection became established
	if( mSocket.ConnectionEstablished() && 
		mTcpConnectionEstablishedTime != -1 &&
		!HandshakeReceived() && 
		((mTimer.ElapsedMs() - mTcpConnectionEstablishedTime) >= HANDSHAKE_TIMEOUT_MS) )
	{
		LogNote(mTimer.ElapsedMs(), "Handshake timed out\n");
		return false;
	}


	// We have had outstanding requests for too long, sack off this peer
	if( HandshakeReceived() &&
		HasOutstandingBlocksToDownload() && 
		((mTimer.ElapsedMs() - mLastPieceRequestTime) >= REQUEST_TIMEOUT) &&
		((mTimer.ElapsedMs() - mLastPieceArriveTime) >= REQUEST_TIMEOUT) )
	{
		char errMsg[64];
		sprintf(errMsg, "Waited %d minutes for a request.\nAssuming dead connection\n", REQUEST_TIMEOUT/(1000*60));
		LogNote(mTimer.ElapsedMs(), errMsg);
		return false;
	}

	// We've been connected for some time and yet we are not interested, sack off
	if( HandshakeReceived() &&
		AmInterested() == false &&
	   ((ConnectionLength() - mLastTimeWeWereInterestedInPeer) >= NOT_INTERESTED_DISCONNECT_PERIOD))
	   
	{
		return false;
	}

	return true;
}// END IsPeerUsable



void cTorrentPeer::CheckIsSeed()
{
	for(u32 i=0; i < mPiecesBitField.Size(); i++)
	{
		if(mPiecesBitField.Get(i) == false)
		{
			return;
		}
	}
	mIsSeed = true;
}// END CheckIsSeed



float cTorrentPeer::PercentageDone() const
{
	u32 numPiecesHave=0;
	u32 totalPieces = mPiecesBitField.Size(); 
	for(u32 i=0; i < totalPieces; i++)
	{
		if(mPiecesBitField.Get(i) != 0)
		{
			numPiecesHave++;
		}
	}
	float percentage=0.0f;
	if(numPiecesHave > 0)
	{
		float onePercent = float(totalPieces / 100.0f);
		percentage =  float(numPiecesHave / onePercent);
	}	
	assert(percentage >= 0.0f && percentage <= 100.1f);
	if(percentage > 100.0f) percentage = 100.0f;
	return percentage;
}// END PercentageDone



// Called whenever any message is sent / received

void cTorrentPeer::OnMessageSent()
{
	mLastMessageSent = mTimer.ElapsedMs();
}// END OnMessageSent



void cTorrentPeer::OnMessageReceived()
{
	mLastMessageReceived = mTimer.ElapsedMs();
}// END OnMessageReceived



void cTorrentPeer::OnHandshakeReceived(const cTorrentHandshakeMessage& msg)
{	
	memcpy(mPeerId, msg.mPeerId, PEER_ID_LENGTH);
	//Printf("got handshake for peer %X%X%X%X%X\n", static_cast<u32>(mPeerId[0]), static_cast<u32>(mPeerId[4]), static_cast<u32>(mPeerId[8]), static_cast<u32>(mPeerId[12]), static_cast<u32>(mPeerId[16]));
	mHandshakeReceived=true;

	LogStandardMessage(0, &msg, cTorrentPacketLog::HANDSHAKE, PTYPE_RECEIVE);
}// END OnHandshakeReceived



void cTorrentPeer::OnBitfieldReceived(const cBitField& bf)
{	
	SetBitfield(bf);
	mBitfieldReceived = true;
	CheckIsSeed();
}// END OnHandshakeReceived



void cTorrentPeer::OnHavePiece(u32 piece)
{
	mPiecesBitField.Set(piece);
	CheckIsSeed();
}// END OnHavePiece



bool cTorrentPeer::RequestBlock(cTorrentPiece* pPiece, u32 blockIndex)
{
	assert(pPiece && mDownloadRequestQueue.GetBlockRequest(pPiece->PieceNumber(), blockIndex)==NULL);
	if(NumberOfOutstandingDownloadBlocks() >= MAX_OUTSTANDING_REQUESTS)
	{
		assert(0);
		return false;
	}

	bool isFinalBlock = (blockIndex == pPiece->NumberOfBlocks() - 1);
	const u32 NEXT_REQUEST_SIZE = (isFinalBlock ? pPiece->FinalBlockSize() : BLOCK_LENGTH);

	//Printf("Piece %d block %d REQUESTED from %s\n", pPiece->PieceNumber(), blockIndex+1, mAddr.Ip().AsString());

	u32 begin = blockIndex*BLOCK_LENGTH;

	// request the next block
	SendRequestMessage(*this, pPiece->PieceNumber(), begin, NEXT_REQUEST_SIZE);

	pPiece->SetBlockStatus(blockIndex, cTorrentPiece::BLOCK_REQUESTED, mTimer.ElapsedMs(), Address());

	cRequestQueue::BlockRequest req;
	req.mDownloadPiece = pPiece;
	req.mPieceNumber = pPiece->PieceNumber();
	req.mBlockNumber = blockIndex;
	req.mBlockRequestTime = mTimer.ElapsedMs();
	req.mBlockBegin = begin;
	req.mBlockSize = NEXT_REQUEST_SIZE;
	bool ret = mDownloadRequestQueue.AddRequest(req);
	assert(ret && mDownloadRequestQueue.Size() <= MAX_OUTSTANDING_REQUESTS);

	mLastPieceRequestTime = mTimer.ElapsedMs();

	return true;
}// END RequestBlock


/*
void cTorrentPeer::CancelCurrentRequest()
{
	assert(mDownloadPiece!=NULL);

	SendCancelMessage(*this, mDownloadPiece->PieceNumber(), mBlockNumberDownloading*BLOCK_LENGTH, mBlockSize);

	mDownloadPiece->SetBlockStatus(mBlockNumberDownloading, cTorrentPiece::BLOCK_MISSING, 0, cSockAddr());
	mBlockNumberDownloading = INVALID_BLOCK_NUMBER;
	mBlockSize = INVALID_BLOCK_NUMBER;
	mDownloadPiece = NULL;
	mIsDownloadingBlock = false;
	mBlockRequestTime = 0;
	mCancelledLastBlock = true;
}// END CancelCurrentRequest
*/


void cTorrentPeer::CancelOutstandingRequest(u32 pieceNumber, u32 blockIndex)
{
	cRequestQueue::BlockRequest* pReq = mDownloadRequestQueue.GetBlockRequest(pieceNumber, blockIndex);
	if(!pReq)
	{
		assert(0);
		return;
	}
	SendCancelMessage(*this, pReq->mDownloadPiece->PieceNumber(), pReq->mBlockNumber*BLOCK_LENGTH, pReq->mBlockSize);
	mDownloadRequestQueue.RemoveBlockRequest(pieceNumber, blockIndex, cTorrentPiece::BLOCK_MISSING);
}// END CancelOutstandingRequest


/*
cTorrentPeer::BlockRequest* cTorrentPeer::GetBlockRequest(u32 pieceNumber, u32 blockNumber)
{
	for(u32 i=0; i < mOutstandingRequests.size(); i++)
	{
		BlockRequest& req = mOutstandingRequests[i];

		if(req.mDownloadPiece->PieceNumber() == pieceNumber &&
		   req.mBlockNumber == blockNumber)
		{
			return &req;
		}
	}
	return NULL;
}// END GetBlockRequest



void cTorrentPeer::RemoveBlockRequest(u32 pieceNumber, u32 blockNumber, cTorrentPiece::BlockStatus blockStatus)
{
	for(RequestVectorIterator iter= mOutstandingRequests.begin(); iter != mOutstandingRequests.end(); iter++)
	{
		const BlockRequest& req = *iter;

		if(req.mDownloadPiece->PieceNumber() == pieceNumber &&
		   req.mBlockNumber == blockNumber)
		{
			req.mDownloadPiece->SetBlockStatus(req.mBlockNumber, blockStatus, mTimer.ElapsedMs(), cSockAddr());

			mOutstandingRequests.erase(iter);
			return;
		}
	}
}// END RemoveBlockRequest



void cTorrentPeer::RemoveAllBlockRequests(bool sendCancelMessage)
{
	while(mOutstandingRequests.size() > 0)
	{
		BlockRequest& req = *(mOutstandingRequests.begin());

		if(sendCancelMessage)
		{
			CancelOutstandingRequest(req.mDownloadPiece->PieceNumber(), req.mBlockNumber);
		}
		else
		{
			RemoveBlockRequest(req.mDownloadPiece->PieceNumber(), req.mBlockNumber, cTorrentPiece::BLOCK_MISSING);
		}
	}
}// END RemoveAllBlockRequests



bool cTorrentPeer::HasAnyOustandingRequestsForPiece(u32 pieceNumber) const 
{ 
	for(u32 i=0; i < mOutstandingRequests.size(); i++)
	{
		const BlockRequest& req = mOutstandingRequests[i];

		if(req.mDownloadPiece->PieceNumber() == pieceNumber)
		{
			return true;
		}
	}
	return false;
}// END HasAnyOustandingRequestsForPiece
*/


void cTorrentPeer::OnBlockDownloaded(cBitTorrent* pTorrent, u32 pieceIndex, u32 begin, u32 size, u32 blockNumber)
{
	//u32 blockNumber = mDownloadPiece->BlockIndexFromOffset(begin);
	
	cRequestQueue::BlockRequest* pReq = mDownloadRequestQueue.GetBlockRequest(pieceIndex, blockNumber);
	if(!pReq)
	{
		assert(0);
		return;
	}
	
	mLifetimeBytesDownloaded += size;

	pReq->mDownloadPiece->SetBlockStatus(blockNumber, cTorrentPiece::BLOCK_DOWNLOADED, 0, cSockAddr());

	//Printf("Piece %d block %d / %d DOWNLOADED from %s\n", pieceIndex, blockNumber+1, pReq->mDownloadPiece->NumberOfBlocks(), mAddr.Ip().AsString());

	mDownloadRequestQueue.RemoveBlockRequest(pieceIndex, blockNumber, cTorrentPiece::BLOCK_DOWNLOADED);

	cPeerMetaData* meta = pTorrent->PeerMetaData(mAddr);
	if(meta)
	{
		meta->OnBytesDownloaded(size);
	}

	pTorrent->OnBlockDownloaded(pieceIndex, blockNumber, size);

	mLastPieceArriveTime = mTimer.ElapsedMs();

	mBytesDownloadedInLastSecond += size;
}// END OnBlockDownloaded



void cTorrentPeer::OnBlockRequested(cBitTorrent* pTorrent, u32 pieceIndex, u32 begin, u32 size)
{
	if(AmChoking())
	{
		return;
	}

	// Some santiy checking the request
	if(pieceIndex > pTorrent->FileSet().NumberOfPieces() ||
	   begin >= pTorrent->FileSet().PieceSize() ||
	   size > pTorrent->FileSet().PieceSize())
	{
		return;
	}

	cRequestQueue::BlockRequest req;
	req.mDownloadPiece = NULL;	
	req.mPieceNumber = pieceIndex;
	req.mBlockRequestTime = mTimer.ElapsedMs();
	req.mBlockBegin = begin;
	req.mBlockSize = size;
	// TODO : wrong
	req.mBlockNumber = 0;
	mUploadRequestQueue.AddRequest(req);
	
	// TODO : how many are we willing to have (not an assert)
	//assert(mUploadRequestQueue.Size() <= );
}// END OnBlockRequested



// we have sent this block to a peer
void cTorrentPeer::OnBlockUploaded(cBitTorrent* pTorrent, u32 pieceIndex, u32 begin, u32 size, u32 blockNumber)
{
	mBytesUploadedInLastSecond += size;
	mLifetimeBytesUploaded += size;
	mUploadRequestQueue.RemoveBlockRequest(pieceIndex, begin, size, cTorrentPiece::BLOCK_DOWNLOADED);
	pTorrent->OnBlockUploaded(pieceIndex, blockNumber, size);
}// END OnBlockUploaded



bool cTorrentPeer::SendUploadRequest(cBitTorrent* pTorrent)
{
	if(mUploadRequestQueue.Size() == 0)
	{
		assert(0);
		return false;
	}

	cRequestQueue::BlockRequest* pReq = mUploadRequestQueue.OldestRequest();
	u32 pieceNumber = pReq->mPieceNumber;
	u32 begin = pReq->mBlockBegin;
	u32 size = pReq->mBlockSize;

	u32 blockNumber = begin / BLOCK_LENGTH;

	u8* buf = new u8[size];
	pTorrent->FileSet().ReadWritePiece(cFilePieceSet::READ, pieceNumber, begin, size, buf);

	bool ret = SendPieceMessage(pTorrent, *this, pieceNumber, begin, size, buf);

	delete[] buf;

	if(ret)
	{
		OnBlockUploaded(pTorrent, pieceNumber, begin, size, blockNumber);
	}

	return ret;
}// END SendUploadRequest



bool cTorrentPeer::IsReadyToDownloadBlock() const
{
	// the blocks allowed to be outstanding are based on bandwidth
	u32 blocksAllowedOutstanding = MAX_OUTSTANDING_REQUESTS;
	if(DownloadRate() <= 1024)
	{
		blocksAllowedOutstanding = 1;
	}
	else
	{
		blocksAllowedOutstanding = min(((DownloadRate() / 1024)/2), MAX_OUTSTANDING_REQUESTS);
	}

	return (mSocket.ConnectionEstablished() && 
		HandshakeReceived() && 
		AmInterested() && 
		!IsChokingMe() &&
		// TODO : change this for peers we are unsure of so only 1 block is requested
		NumberOfOutstandingDownloadBlocks() < blocksAllowedOutstanding);
}// END IsReadyToDownloadBlock



void cTorrentPeer::AmChoking(bool b) 
{ 
	mAmChoking = b; 

	if(mAmChoking)
	{
		mUploadRequestQueue.RemoveAllBlockRequests(false);
	}
}// END AmChoking



void cTorrentPeer::IsChokingMe(bool b) 
{ 
	mIsChokingMe = b; 

	if(mIsChokingMe)
	{
		mPeerChokedUsTime = mTimer.ElapsedMs();

		// what if we have a request outstanding (does happen, seen in log), passive cancel i recon
		if(HasOutstandingBlocksToDownload())
		{
			OutstandingRequestQueue().RemoveAllBlockRequests(false);
		}
	}
	else
	{
		mPeerChokedUsTime = 0;
	}
}// END IsChokingMe



void cTorrentPeer::AmInterested(bool b) 
{ 
	mAmInterested = b; 

	if(mAmInterested)
	{
		mLastTimeWeWereInterestedInPeer = mTimer.ElapsedMs();
	}
}// END AmInterested