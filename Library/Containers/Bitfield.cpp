// Jon Bellamy 24/02/2009
// Dynamically sizable bitfield
// Note that bit indices run from left to right, therefore is you do :
// cBitFiled bf(8); bf.Set(0);
// mpStorage will equal 1000 0000

// Bits Per Value must a power of 2 and can be a max of 8

#include "Bitfield.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#include "Math/MathHelpers.h"
#include "Network/Base64.h"
#include "File/file.h"



cBitField::cBitField()
: mNumValues(0)
, mBitsPerValue(0)
, mBytesRequired(0)
, mpStorage(NULL)
{
}// END cBitField



cBitField::cBitField(u32 numValues, u32 bitsPerValue)
: mNumValues(0)
, mBitsPerValue(0)
, mpStorage(NULL)
{
	Resize(numValues, bitsPerValue);
}// END cBitField



// copy constructor
cBitField::cBitField(const cBitField& rhs)
: mpStorage(NULL)
, mNumValues(0)
, mBitsPerValue(0)
{
	*this = rhs;
}// END cBitField



cBitField::~cBitField()
{
	Free();
}// END ~cBitField



void cBitField::Resize(u32 numValues, u32 bitsPerValue)
{
	// Bits Per Value must a power of 2 and can be a max of 8
	assert(numValues > 0 && bitsPerValue > 0 && bitsPerValue <= 8 && IsPow2(bitsPerValue));

	Free();
	if(numValues > 0 && bitsPerValue > 0 && bitsPerValue <= 8 && IsPow2(bitsPerValue))
	{
		mNumValues = numValues;
		mBitsPerValue = bitsPerValue;
		CalcBytesRequired();
		u32 numBytes = BytesRequired();
		mpStorage = new u8 [numBytes];
		Zero();
	}
}// END Resize



void cBitField::SetFromData(u32 numValues, u32 bitsPerValue, const u8* pBits)
{
	Free();
	mNumValues = numValues;
	mBitsPerValue = bitsPerValue;
	CalcBytesRequired();
	u32 numBytes = BytesRequired();
	mpStorage = new u8 [numBytes];
	memcpy(mpStorage, pBits, numBytes);
}// END SetFromData



const cBitField& cBitField::operator= (const cBitField& rhs)
{
	Free();
	mNumValues = rhs.mNumValues;
	mBitsPerValue = rhs.mBitsPerValue;
	u32 numBytes = rhs.BytesRequired();
	mpStorage = new u8 [numBytes];
	memcpy(mpStorage, rhs.mpStorage, numBytes);
	return *this;
}// END operator=



bool cBitField::operator== (const cBitField& rhs) const
{
	if(mNumValues != rhs.mNumValues || 
	   mBitsPerValue != rhs.mBitsPerValue)
	{
		return false;
	}
	return (memcmp(mpStorage, rhs.mpStorage, BytesRequired()) == 0);
}// END operator==



u32 cBitField::operator[] (u32 index) const
{
	return Get(index);
}// END operator[]



// index 0 is the most significant bit of the first byte, ie the leftmost bit
void cBitField::Set(u32 valueIndex, u32 value)
{
	if(!mpStorage || valueIndex >= mNumValues)
	{
		assert(0);
		return;
	}

	u8 MASK = (1 << mBitsPerValue)-1;
	u8* pByte = &mpStorage[StorageIndex(valueIndex)];

	// value is too big
	assert(value <= u32((1 << mBitsPerValue)-1));

	Clear(valueIndex);

	// insert the new value
	u8 insertValue = value << ((8-mBitsPerValue) - StartBit(valueIndex));
	*pByte |= insertValue;
}// END Set



void cBitField::Clear(u32 valueIndex)
{
	if(!mpStorage || valueIndex >= mNumValues)
	{
		assert(0);
		return;
	}

	u8* pByte = &mpStorage[StorageIndex(valueIndex)];
	u8 MASK = (1 << mBitsPerValue)-1;
	u8 SHIFTED_MASK = MASK << ((8-mBitsPerValue) - StartBit(valueIndex));

	// clear the value
	*pByte &= ~SHIFTED_MASK;
}// END Clear



u32 cBitField::Get(u32 valueIndex) const
{
	if(!mpStorage || valueIndex >= mNumValues)
	{
		assert(0);
		return false;
	}
	
	u8 b = mpStorage[StorageIndex(valueIndex)];
	u8 MASK = (1 << mBitsPerValue)-1;
	u8 SHIFTED_MASK = MASK << ((8-mBitsPerValue) - StartBit(valueIndex));

	b &= SHIFTED_MASK;
	
	return b >> ((8-mBitsPerValue) - StartBit(valueIndex));
}// END Get



void cBitField::Zero()
{
	if(mpStorage && mNumValues > 0 && mBitsPerValue > 0)
	{
		memset(mpStorage, 0, BytesRequired());
	}
}// END Zero



void cBitField::Free()
{
	mNumValues = 0;
	mBitsPerValue = 0;
	mBytesRequired = 0;
	if(mpStorage)
	{
		delete[] mpStorage;
		mpStorage = NULL;
	}
}// END Free



void cBitField::CalcBytesRequired()
{
	double d = ceil((double(mNumValues) * double(mBitsPerValue) / 8));
	mBytesRequired = __max(1, static_cast<u32>(d));
}// END CalcBytesRequired



// which byte does the passed value reside
u32 cBitField::StorageIndex(u32 valueIndex) const 
{ 
	assert(valueIndex < mNumValues); 
	return (valueIndex * mBitsPerValue) / 8;
}// END StorageIndex




// which bit does the passed value start at
u32 cBitField::StartBit(u32 valueIndex) const 
{ 
	return((valueIndex * mBitsPerValue) % 8);
}// END StartBit



std::string cBitField::ToBase64() const
{
	u32 bufferSize = max(64, ((BytesRequired() * 3) + (sizeof(mNumValues) + sizeof(mBitsPerValue))));
	u8* pBase64 = new u8[bufferSize];
	u8* pSourceBuffer = new u8[bufferSize];
	
	u32 offset = sizeof(mNumValues) + sizeof(mBitsPerValue);
	memcpy(pSourceBuffer, &mNumValues, sizeof(mNumValues));
	memcpy(pSourceBuffer + sizeof(mNumValues), &mBitsPerValue, sizeof(mBitsPerValue));
	memcpy(pSourceBuffer + offset, mpStorage, BytesRequired());

	int numBytes = net::EncodeBase64(pSourceBuffer, BytesRequired() + offset, pBase64, bufferSize);

	std::string base64Str;
	base64Str.insert(0, reinterpret_cast<const char*> (pBase64), numBytes);

	delete[] pBase64;
	delete[] pSourceBuffer;

	return base64Str;
}// END ToBase64



void cBitField::FromBase64(const std::string& b64EncodedBitfield)
{
	u32 bufferSize = b64EncodedBitfield.size();
	u8* pDecodeBuf = new u8[bufferSize];
	net::DecodeBase64(reinterpret_cast<const u8*> (b64EncodedBitfield.c_str()), b64EncodedBitfield.size(), pDecodeBuf, bufferSize);
	
	memcpy(&mNumValues, pDecodeBuf, sizeof(mNumValues));
	memcpy(&mBitsPerValue, pDecodeBuf + sizeof(mNumValues), sizeof(mBitsPerValue));
	CalcBytesRequired();

	SetFromData(mNumValues, mBitsPerValue, pDecodeBuf + sizeof(mNumValues) + sizeof(mBitsPerValue));

	delete[] pDecodeBuf;
}// END FromBase64



void cBitField::WriteToDisk(const char* fn)
{
	cFile f(cFile::WRITE, fn);
	f.Write(0, 4, reinterpret_cast<u8*>(&mNumValues));
	f.Write(4, 4, reinterpret_cast<u8*>(&mBitsPerValue));
	f.Write(8, BytesRequired(), mpStorage);
}// END WriteToDisk



bool cBitField::LoadFromDisk(const char* fn)
{
	cFile f(cFile::READ, fn, true);
	if(f.Size() < 9)
	{
		return false;
	}
	bool bRet;
	bRet = f.Read(0, 4, reinterpret_cast<u8*>(&mNumValues));
	if(!bRet)
	{
		return false;
	}
	bRet = f.Read(4, 4, reinterpret_cast<u8*>(&mBitsPerValue));
	if(!bRet)
	{
		return false;
	}
	if((f.Size()-8) != BytesRequired())
	{
		return false;
	}
	SetFromData(mNumValues, mBitsPerValue, f.CachedFileData()+8);	
	return true;
}// END LoadFromDisk