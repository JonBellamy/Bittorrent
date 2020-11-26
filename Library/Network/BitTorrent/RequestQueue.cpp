
#include "RequestQueue.h"

#include "Network/SockAddr.h"
#include "Network/BitTorrent/BitTorrentPeer.h"

using namespace net;


cRequestQueue::cRequestQueue()
: mpPeer(NULL)
{
}// END cRequestQueue




cRequestQueue::~cRequestQueue()
{
	RemoveAllBlockRequests(false);
	mOutstandingRequests.clear();
}// END ~cRequestQueue



bool cRequestQueue::AddRequest(const BlockRequest& req)
{
	if(GetBlockRequest(req.mPieceNumber, req.mBlockBegin, req.mBlockSize))
	{
		Printf("Request for block that we already have requested\n");
		return false;
	}
	mOutstandingRequests.push_back(req);
	return true;
}// END AddRequest



cRequestQueue::BlockRequest* cRequestQueue::GetBlockRequest(u32 pieceNumber, u32 blockNumber)
{
	for(u32 i=0; i < mOutstandingRequests.size(); i++)
	{
		BlockRequest& req = mOutstandingRequests[i];

		if(req.mPieceNumber == pieceNumber &&
		   req.mBlockNumber == blockNumber)
		{
			return &req;
		}
	}
	return NULL;
}// END GetBlockRequest



cRequestQueue::BlockRequest* cRequestQueue::GetBlockRequest(u32 pieceNumber, u32 begin, u32 size)
{
	for(u32 i=0; i < mOutstandingRequests.size(); i++)
	{
		BlockRequest& req = mOutstandingRequests[i];

		if(req.mPieceNumber == pieceNumber &&
		   req.mBlockBegin == begin &&
		   req.mBlockSize == size)
		{
			return &req;
		}
	}
	return NULL;
}// END GetBlockRequest




void cRequestQueue::RemoveBlockRequest(u32 pieceNumber, u32 blockNumber, cTorrentPiece::BlockStatus blockStatus)
{
	for(RequestVectorIterator iter = mOutstandingRequests.begin(); iter != mOutstandingRequests.end(); iter++)
	{
		const BlockRequest& req = *iter;

		if(req.mPieceNumber == pieceNumber &&
		   req.mBlockNumber == blockNumber)
		{
			if(req.mDownloadPiece)
			{
				req.mDownloadPiece->SetBlockStatus(req.mBlockNumber, blockStatus, 0, cSockAddr());
			}
			mOutstandingRequests.erase(iter);
			return;
		}
	}
}// END RemoveBlockRequest



void cRequestQueue::RemoveBlockRequest(u32 pieceNumber, u32 begin, u32 size, cTorrentPiece::BlockStatus blockStatus)
{
	for(RequestVectorIterator iter= mOutstandingRequests.begin(); iter != mOutstandingRequests.end(); iter++)
	{
		const BlockRequest& req = *iter;

		if(req.mPieceNumber == pieceNumber &&
		   req.mBlockBegin == begin &&
		   req.mBlockSize == size)
		{
			if(req.mDownloadPiece)
			{
				req.mDownloadPiece->SetBlockStatus(req.mBlockNumber, blockStatus, 0, cSockAddr());
			}

			mOutstandingRequests.erase(iter);
			return;
		}
	}
}// END RemoveBlockRequest



void cRequestQueue::RemoveAllBlockRequests(bool sendCancelMessage)
{
	while(mOutstandingRequests.size() > 0)
	{
		BlockRequest& req = *(mOutstandingRequests.begin());

		if(sendCancelMessage)
		{
			mpPeer->CancelOutstandingRequest(req.mPieceNumber, req.mBlockNumber);
		}
		else
		{
			RemoveBlockRequest(req.mPieceNumber, req.mBlockBegin, req.mBlockSize, cTorrentPiece::BLOCK_MISSING);
		}
	}
}// END RemoveAllBlockRequests



void cRequestQueue::CancelAllPieceRequests(u32 pieceNumber)
{
	bool sentACancel;
	do 
	{
		sentACancel=false;

		for(u32 i=0; i < mOutstandingRequests.size(); i++)
		{
			BlockRequest& req = mOutstandingRequests[i];

			if(req.mPieceNumber == pieceNumber)
			{
				sentACancel = true;
				mpPeer->CancelOutstandingRequest(pieceNumber, req.mBlockNumber);
				break;
			}
		}
	} 
	while (sentACancel);
}// END CancelAllPieceRequests



bool cRequestQueue::HasAnyOustandingRequestsForPiece(u32 pieceNumber) const 
{ 
	for(u32 i=0; i < mOutstandingRequests.size(); i++)
	{
		const BlockRequest& req = mOutstandingRequests[i];

		if(req.mPieceNumber == pieceNumber)
		{
			return true;
		}
	}
	return false;
}// END HasAnyOustandingRequestsForPiece