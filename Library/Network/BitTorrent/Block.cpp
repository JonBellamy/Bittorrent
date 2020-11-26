
#include "block.h"

#include <math.h>
#include <memory.h>
#include "Math/MathHelpers.h"



cTorrentBlock::cTorrentBlock()
: mPieceIndex(0xFFFFFFFF)
, mBegin(0xFFFFFFFF)
, mSize(0xFFFFFFFF)
{
}// END cTorrentBlock



cTorrentBlock::cTorrentBlock(const cTorrentBlock& rhs)
{
	*this = rhs;
}// END cTorrentPiece



cTorrentBlock::~cTorrentBlock()
{
}// END ~cTorrentBlock



const cTorrentBlock& cTorrentBlock::operator= (const cTorrentBlock& rhs)
{
	Set(rhs.mPieceIndex, rhs.mBegin, rhs.mSize);
	return *this;
}// END operator=



void cTorrentBlock::Set(u32 pieceIndex, u32 begin, u32 size)
{
	mPieceIndex = pieceIndex;
	mBegin = begin;
	mSize = size;
}// END Reset


