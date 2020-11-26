
#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <assert.h>
#include <string>


class cByteStream
{
public:

	enum
	{
		DEFAULT_START_CAPACITY = 512
	};

	cByteStream(u32 capcacity=DEFAULT_START_CAPACITY, bool canExpand=true);
	cByteStream(const cByteStream& rhs);
	virtual ~cByteStream();


	const cByteStream& operator= (const cByteStream& rhs);
	bool operator== (const cByteStream& rhs) const;

	u8& operator[] (u32 index);	
	const u8& operator[] (u32 index) const;

	void Clear(bool zeroMemory=true);
	void ClearAndResize(u32 newCapacity=DEFAULT_START_CAPACITY, bool resizeToNextPowerOfTwo=false);
	void Resize(u32 newCapacity, bool resizeToNextPowerOfTwo=false) { Expand(newCapacity, resizeToNextPowerOfTwo); }
	void DoubleSize() { Resize(Capacity()*2, true); }
	void ZeroUnused();

	// removes all free space from the array
	void Trim();

	bool StreamBytes(const u8* newData, u32 numBytes);

	// the byte at startIndex is removed
	void RemoveBytes(u32 startIndex, u32 numBytes);

	void GetBytes(u8* outBuffer, u32 startIndex, u32 numBytes);

	// searches for the passed string of bytes, if successful returns the start index of the bytes otherwise -1
	s32 Find(const char* bytes, u32 numBytes) const;

	u8 ReadU8(u32 index);
	u16 ReadU16(u32 index);
	u32 ReadU32(u32 index);

	u8* Data() { return mData; }
	const u8* Data() const { return mData; }

	// this is used at the cTcpSocketLevel to transparently place bytes in the byte stream. Do not use it elsewhere.
	void BytesStreamedManually(u32 numbytes) { assert(mSize+numbytes <= Capacity()); mSize += numbytes; }

	u32 Size() const { return mSize; }
	u32 Capacity() const { return mCapcacity; }
	u32 FreeSpace() const { return mCapcacity - mSize; }


	std::string AsString() const { return std::string(reinterpret_cast<char*>(mData)); }

	bool CanExpand() const { return mCanExpand; }
	void CanExpand(bool b) { mCanExpand = b; }

private:

	void Expand(u32 capcacity, bool expandToNextPowerOfTwo=false);

	u32 NextPowerOfTwo(u32 val) const;

	u8* mData;
	u32 mSize;
	u32 mCapcacity;

	bool mCanExpand;
};





#endif 
