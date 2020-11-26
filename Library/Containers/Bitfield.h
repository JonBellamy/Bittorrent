// Jon Bellamy 24/02/2009
// Dynamically sizable bitfield
// Note that bit indices run from left to right, therefore is you do :
// cBitFiled bf(8); bf.Set(0);
// mpStorage will equal 1000 0000

// Bits Per Value must a power of 2 and can be a max of 8


#ifndef _BITFIELD_DYN_H
#define _BITFIELD_DYN_H

#if USE_PCH
#include "stdafx.h"
#endif


#include <string>


class cBitField
{
public:

	// Bits Per Value must a power of 2 and can be a max of 8
	cBitField();
	cBitField(u32 numValues, u32 bitsPerValue);
	cBitField(const cBitField& rhs);
	virtual ~cBitField();

	void Resize(u32 numValues, u32 bitsPerValue);

	void SetFromData(u32 numValues, u32 bitsPerValue, const u8* pBits);

	const cBitField& operator= (const cBitField& rhs);	
	bool operator== (const cBitField& rhs) const;
	u32 operator[] (u32 index) const;

	// index 0 is the most significant bit of the first byte, ie the leftmost bit
	void Set(u32 valueIndex, u32 value=1);
	void Clear(u32 valueIndex);
	u32 Get(u32 valueIndex) const;

	void Zero();

	u32 BytesRequired() const;
	const u8* Storage() const { return mpStorage; }

	// in bits / values ...
	u32 Size() const { return mNumValues; }

	std::string ToBase64() const;
	void FromBase64(const std::string& b64EncodedBitfield);

	void WriteToDisk(const char* fn);
	bool LoadFromDisk(const char* fn);

private:

	void CalcBytesRequired();

	void Free();

	// which byte does the passed bit index reside in
	u32 StorageIndex(u32 valueIndex) const;
	u32 StartBit(u32 valueIndex) const;

	u8* mpStorage;
	u32 mBytesRequired;		// This is really just for debug

	u32 mNumValues;
	u32 mBitsPerValue;
};



inline u32 cBitField::BytesRequired() const
{
	return mBytesRequired;
}// END BytesRequired




#endif // _BITFIELD_DYN_H
