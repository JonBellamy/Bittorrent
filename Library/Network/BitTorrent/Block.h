
#ifndef TORRENT_BLOCK_H
#define TORRENT_BLOCK_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <assert.h>

#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/SockAddr.h"
#include "Containers/Bitfield.h"



class cTorrentBlock
{
public:
    cTorrentBlock();
	cTorrentBlock(const cTorrentBlock&);
	~cTorrentBlock();

	const cTorrentBlock& operator= (const cTorrentBlock& rhs);

public:

	void Set(u32 pieceIndex, u32 begin, u32 size);

	u32 PieceNumber() const { return mPieceIndex; }
	u32 Begin() const { return mBegin; }
	u32 Size() const { return mSize; }

private:

	u32 mPieceIndex;
	u32 mBegin;
	u32 mSize;
};




#endif // TORRENT_BLOCK_H
