
#ifndef BYTESTREAM_H
#define BYTESTREAM_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <string>


class cByteStream
{
public:

	enum
	{
		START_CAPACITY = 2048
	};

	cByteStream(u32 capcacity=START_CAPACITY);
	cByteStream(const cByteStream& rhs);
	virtual ~cByteStream();


	const cByteStream& operator= (const cByteStream& rhs);
	bool operator== (const cByteStream& rhs) const;

	u8& operator[] (u32 index);

	void Clear();
	void ClearAndResize(u32 newCapacity=START_CAPACITY);

	// removes all free space from the array
	void Trim();

	void StreamBytes(const u8* newData, u32 numBytes);

	// the byte at startIndex is removed
	void RemoveBytes(u32 startIndex, u32 numBytes);

	void GetBytes(u8* outBuffer, u32 startIndex, u32 numBytes);

	// searches for the passed string of bytes, if succesful returns the start index of the bytes otherwise -1
	s32 Find(const char* bytes, u32 numBytes) const;

	u8* Data() { return mData; }
	const u8* Data() const { return mData; }

	u32 Size() const { return mSize; }
	u32 Capacity() const { return mCapcacity; }

	std::string AsString() const { return std::string(reinterpret_cast<char*>(mData)); }


private:

	void Expand(u32 capcacity);

	u8* mData;
	u32 mSize;
	u32 mCapcacity;
};





#endif 
