// Jon Bellamy 21/02/2009
// We download pieces in smaller blocks, this keeps track of our progress on a piece

#ifndef TORRENT_PIECE_H
#define TORRENT_PIECE_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <assert.h>

#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/SockAddr.h"
#include "Containers/Bitfield.h"



class cTorrentPiece
{
public:
    cTorrentPiece();
	cTorrentPiece(const cTorrentPiece&);
	cTorrentPiece(u32 pieceNumber, u32 pieceSize);
	~cTorrentPiece();

	const cTorrentPiece& operator= (const cTorrentPiece& rhs);

public:

	void Reset(u32 pieceNumber, u32 pieceSize);

	bool IsValid() const;

	bool HasDownloaded() const;
	
	// have all the blocks for this piece been requested
	bool AllBlocksPending() const;

	u32 NumberOfBlocks() const;

	u32 PieceSize() const;

	//void SetPieceNumber(u32 piece) { mPieceNumber = piece; }
	u32 PieceNumber() const { return mPieceNumber; }

	u32 NextMissingBlockIndex() const;

	// pieceOffset must be the start of a block
	u32 BlockIndexFromOffset(u32 pieceOffset) const;

	u32 FinalBlockSize() const;

	u32 BlockRequestTime(u32 blockIndex) const { assert(mBlockMetaData); return mBlockMetaData[blockIndex].mRequestTime; }

	u32 NumberOfCompletedBlocks() const;
	u32 NumberOfOutstandingBlocks() const;
	u32 NumberOfRequestedBlocks() const;

	enum
	{
		INVALID_PIECE_NUMBER = 0xFFFFFFFF
	};

	typedef enum
	{
		BLOCK_MISSING		= 0,
		BLOCK_REQUESTED		= 1,		
		BLOCK_DOWNLOADING	= 2,
		BLOCK_DOWNLOADED	= 3
	}BlockStatus;

	void SetBlockStatus(u32 blockIndex, BlockStatus status, u32 time, net::cSockAddr peerAddr); 
	BlockStatus GetBlockStatus(u32 blockIndex) const; 

	void Clear();

	void SetBitfieldFromBase64Str(const std::string& base64Bitfield);
	std::string BitfieldBase64() const;


private:

	u32 BlockStatusCount(BlockStatus status) const;

	u32 mPieceNumber;
	u32 mNumberOfBlocks;
	u32 mPieceSize;
	cBitField mBlocksDownloadStatus;

	struct sBlockMetaData
	{
	public:
		sBlockMetaData() : mRequestTime(0) {}
		sBlockMetaData(const sBlockMetaData& rhs)	{ *this = rhs; }

		const sBlockMetaData& operator= (const sBlockMetaData& rhs)
		{
			mRequestTime = rhs.mRequestTime;
			mPeerAddr = rhs.mPeerAddr;
			return *this;
		}

		u32 mRequestTime;
		net::cSockAddr mPeerAddr;
	};
	sBlockMetaData* mBlockMetaData;
};



inline void cTorrentPiece::Clear() 
{ 
	mPieceNumber = INVALID_PIECE_NUMBER; 
	mBlocksDownloadStatus.Zero(); 
	if(mBlockMetaData)
	{
		delete[] mBlockMetaData;
		mBlockMetaData = NULL;
	}
}// END Clear



inline bool cTorrentPiece::IsValid() const
{
	return mPieceNumber != INVALID_PIECE_NUMBER;
}// END IsValid



inline u32 cTorrentPiece::NumberOfBlocks() const
{
	return mNumberOfBlocks;
}// END NumberOfBlocks



inline u32 cTorrentPiece::PieceSize() const
{
	return mPieceSize;
}// END PieceSize



// have all the blocks for this piece been requested
inline bool cTorrentPiece::AllBlocksPending() const
{
	for(u32 i=0; i < mBlocksDownloadStatus.Size(); i++)
	{
		if(mBlocksDownloadStatus.Get(i) == BLOCK_MISSING)
		{
			return false;
		}
	}
	return true;
}// END AllBlocksPending



inline void cTorrentPiece::SetBlockStatus(u32 blockIndex, BlockStatus status, u32 time, net::cSockAddr peerAddr)
{
	mBlocksDownloadStatus.Set(blockIndex, status);

	if(status == BLOCK_REQUESTED)
	{
		assert(mBlockMetaData);
		mBlockMetaData[blockIndex].mRequestTime = time;
		mBlockMetaData[blockIndex].mPeerAddr = peerAddr;
	}
}// END SetBlockStatus



inline cTorrentPiece::BlockStatus cTorrentPiece::GetBlockStatus(u32 blockIndex) const
{
	return static_cast<BlockStatus> (mBlocksDownloadStatus.Get(blockIndex));
}// END GetBlockStatus





#endif // TORRENT_PIECE_H
