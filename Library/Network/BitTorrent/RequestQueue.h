
#ifndef REQ_QUEUE_H
#define REQ_QUEUE_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <assert.h>
#include <vector>

#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/BitTorrent/Piece.h"


class cTorrentPeer;


class cRequestQueue
{
public:
    cRequestQueue();
	~cRequestQueue();

private:
	cRequestQueue(const cRequestQueue& rhs);
	const cRequestQueue& operator= (const cRequestQueue& rhs);

public:
	typedef struct 
	{
		cTorrentPiece* mDownloadPiece;
		u32 mPieceNumber;
		u32 mBlockNumber;
		u32 mBlockBegin;
		u32 mBlockSize;
		u32 mBlockRequestTime;
	}BlockRequest;

	void SetPeer(cTorrentPeer* pPeer) { mpPeer = pPeer; }

	bool AddRequest(const BlockRequest& req);

	BlockRequest* GetBlockRequest(u32 pieceNumber, u32 blockNumber);
	BlockRequest* GetBlockRequest(u32 pieceNumber, u32 begin, u32 size);

	void RemoveBlockRequest(u32 pieceNumber, u32 blockNumber, cTorrentPiece::BlockStatus blockStatus);
	void RemoveBlockRequest(u32 pieceNumber, u32 begin, u32 size, cTorrentPiece::BlockStatus blockStatus);
	void RemoveAllBlockRequests(bool sendCancelMessage);

	void CancelAllPieceRequests(u32 pieceNumber);
	bool HasAnyOustandingRequestsForPiece(u32 pieceNumber) const;

	u32 Size() const { return (u32)mOutstandingRequests.size(); }
	BlockRequest* OldestRequest() { assert(Size()>0); return &(mOutstandingRequests[0]); } 

private:
	cTorrentPeer* mpPeer;

	typedef std::vector<BlockRequest> RequestVector;
	typedef RequestVector::iterator RequestVectorIterator;
	RequestVector mOutstandingRequests;
};




#endif // REQ_QUEUE_H
