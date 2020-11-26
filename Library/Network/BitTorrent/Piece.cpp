// Jon Bellamy 21/02/2009


#include "Piece.h"

#include <math.h>
#include <memory.h>
#include "Math/MathHelpers.h"



cTorrentPiece::cTorrentPiece()
: mPieceSize(0)
, mPieceNumber(INVALID_PIECE_NUMBER)
, mNumberOfBlocks(0)
, mBlockMetaData(NULL)
{
}// END cTorrentPiece



cTorrentPiece::cTorrentPiece(const cTorrentPiece& rhs)
: mPieceSize(0)
, mPieceNumber(INVALID_PIECE_NUMBER)
, mNumberOfBlocks(0)
, mBlockMetaData(NULL)
{
	*this = rhs;
}// END cTorrentPiece



cTorrentPiece::cTorrentPiece(u32 pieceNumber, u32 pieceSize)
: mPieceSize(0)
, mPieceNumber(INVALID_PIECE_NUMBER)
, mNumberOfBlocks(0)
, mBlockMetaData(NULL)
{
	Reset(pieceNumber, pieceSize);
}// END cTorrentPiece



cTorrentPiece::~cTorrentPiece()
{
	if(mBlockMetaData)
	{
		delete[] mBlockMetaData;
		mBlockMetaData = NULL;
	}
}// END ~cTorrentBlock



const cTorrentPiece& cTorrentPiece::operator= (const cTorrentPiece& rhs)
{
	Reset(rhs.mPieceNumber, rhs.mPieceSize);
	mBlocksDownloadStatus = rhs.mBlocksDownloadStatus;
	for(u32 i=0; i < mNumberOfBlocks; i++)
	{
		mBlockMetaData[i] = rhs.mBlockMetaData[i];
	}
	return *this;
}// END operator=



void cTorrentPiece::Reset(u32 pieceNumber, u32 pieceSize)
{
	Clear();

	mPieceNumber = pieceNumber;
	mPieceSize = pieceSize;

	//assert(IsPow2(pieceSize));
	double d = ceil(double(pieceSize) / double(BLOCK_LENGTH));
	mNumberOfBlocks = static_cast<u32>(d);
	mBlocksDownloadStatus.Resize(mNumberOfBlocks, 2);

	if(mBlockMetaData)
	{
		delete[] mBlockMetaData;
	}
	mBlockMetaData = new sBlockMetaData[mNumberOfBlocks];
	assert(mBlockMetaData);
	memset(mBlockMetaData, 0, sizeof(sBlockMetaData) * mNumberOfBlocks);
}// END Reset



bool cTorrentPiece::HasDownloaded() const
{
	assert(IsValid());
	for(u32 i=0; i < mBlocksDownloadStatus.Size(); i++)
	{
		if(mBlocksDownloadStatus.Get(i) != BLOCK_DOWNLOADED)
		{
			return false;
		}
	}
	return true;
}// END HasDownloaded



u32 cTorrentPiece::NextMissingBlockIndex() const
{
	assert(mPieceNumber != INVALID_PIECE_NUMBER);
	for(u32 i=0; i < mBlocksDownloadStatus.Size(); i++)
	{
		if(mBlocksDownloadStatus.Get(i) == BLOCK_MISSING)
		{
			return i;
		}
	}

	assert(0);
	return 0;
}// END NextMissingBlockIndex



// pieceOffset must be the start of a block
u32 cTorrentPiece::BlockIndexFromOffset(u32 pieceOffset) const
{
	assert(mPieceNumber != INVALID_PIECE_NUMBER);

	// this assert does not hold for the final block of the final piece
	if(FinalBlockSize() == BLOCK_LENGTH)
	{
		assert(pieceOffset <= (mPieceSize - BLOCK_LENGTH));
	}
	return pieceOffset / BLOCK_LENGTH;
}// END BlockIndexFromOffset



// this will always equal BLOCK_LENGTH unless its the final block of the final piece
u32 cTorrentPiece::FinalBlockSize() const
{
	assert(mPieceNumber != INVALID_PIECE_NUMBER);
	return mPieceSize - ((NumberOfBlocks()-1) * BLOCK_LENGTH);
}// END FinalBlockSize



u32 cTorrentPiece::BlockStatusCount(BlockStatus status) const
{
	u32 count = 0;
	for(u32 i=0; i < mBlocksDownloadStatus.Size(); i++)
	{
		if(mBlocksDownloadStatus.Get(i) == status)
		{
			count++;
		}
	}
	return count;
}// END BlockStatusCount



u32 cTorrentPiece::NumberOfCompletedBlocks() const
{
	return BlockStatusCount(BLOCK_DOWNLOADED);
}// END NumberOfCompletedBlocks



u32 cTorrentPiece::NumberOfOutstandingBlocks() const
{
	return BlockStatusCount(BLOCK_MISSING);
}// END NumberOfOutstandingBlocks



u32 cTorrentPiece::NumberOfRequestedBlocks() const
{
	return BlockStatusCount(BLOCK_REQUESTED) + BlockStatusCount(BLOCK_DOWNLOADING);
}// END NumberOfRequestedBlocks



void cTorrentPiece::SetBitfieldFromBase64Str(const std::string& base64Bitfield)
{
	mBlocksDownloadStatus.FromBase64(base64Bitfield);
	
	// When we save or load a piece the total state is saved so some blocks will be marked as requested, clear them.
	for(u32 i=0; i < mBlocksDownloadStatus.Size(); i++)
	{
		if(mBlocksDownloadStatus.Get(i) == BLOCK_REQUESTED)
		{
			mBlocksDownloadStatus.Set(i, BLOCK_MISSING);
		}
	}
}// END SetBitfieldFromBase64Str



std::string cTorrentPiece::BitfieldBase64() const
{
	return mBlocksDownloadStatus.ToBase64();
}// END BitfieldBase64