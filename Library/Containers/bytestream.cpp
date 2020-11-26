
#include "ByteStream.h"

#include <assert.h>
#include <memory.h>


cByteStream::cByteStream(u32 capcacity)
: mCapcacity(0)
, mSize(0)
, mData(NULL)
{
	Expand(capcacity);
}// END cByteStream



// copy constructor
cByteStream::cByteStream(const cByteStream& rhs)
: mCapcacity(0)
, mSize(0)
, mData(NULL)
{
	*this = rhs;
}// END cByteStream



cByteStream::~cByteStream()
{
	if(mData)
	{
		delete[] mData;
	}
}// END ~cByteStream



const cByteStream& cByteStream::operator= (const cByteStream& rhs)
{
	Expand(rhs.Capacity());
	memcpy(mData, rhs.mData, rhs.Size());
	mSize = rhs.Size();
	return *this;
}// END operator=



bool cByteStream::operator== (const cByteStream& rhs) const
{
	if(Size() != rhs.Size())
	{
		return false;
	}
	return (memcmp(mData, rhs.Data(), Size()) == 0);
}// END operator==



u8& cByteStream::operator[] (u32 index)
{
	// Not 100% sure this is a good idea. You cannot expand the bytestream using this operator but you can 
	// increase its size.
	assert(index < Capacity()-1);
	if(index >= Size())
	{
		mSize = index+1;
	}
	assert(index < Capacity());
	return mData[index];
}// END operator[]



void cByteStream::Clear()
{
	if(mData)
	{
		memset(mData, 0, mCapcacity);
	}
	mSize = 0;
}// END Clear



void cByteStream::ClearAndResize(u32 newCapacity)
{
	Clear();
	Expand(newCapacity);
}// END ClearAndResize



// removes all free space from the array
void cByteStream::Trim()
{
	Expand(Size());
}// END Trim



void cByteStream::StreamBytes(const u8* newData, u32 numBytes)
{
	assert(mData);

	// double the size if needed
	if(mSize + numBytes > mCapcacity)
	{
		u32 newCapacity = Capacity() * 2;
		while(newCapacity < (Size() + numBytes))
		{
			newCapacity *= 2;
		}
		
		Expand(newCapacity);
	}

	memcpy(mData + mSize, newData, numBytes);

	mSize += numBytes;

	assert(mSize <= mCapcacity);
}// END StreamBytes



// the byte at startIndex is removed
void cByteStream::RemoveBytes(u32 startIndex, u32 numBytes)
{
	assert(Size() >= (startIndex + numBytes));
	assert(mData);

	u32 bytesToKeepFront = startIndex;
	u32 bytesToKeepBack = Size() - bytesToKeepFront - numBytes;

	u8* newData = new u8[mCapcacity];
	memset(newData, 0, mCapcacity);
	
	mSize = 0;

	// copy first up to the beginning of the remove part
	if(bytesToKeepFront > 0)
	{
		memcpy(newData, mData, bytesToKeepFront);
		mSize = bytesToKeepFront;
	}

	// copy the rest after the remove part
	if(bytesToKeepBack > 0)
	{
		memcpy(newData + mSize, mData + (startIndex+numBytes), bytesToKeepBack);
		mSize += bytesToKeepBack;
	}

	delete[] mData;
	mData = newData;
}// END RemoveBytes



void cByteStream::GetBytes(u8* outBuffer, u32 startIndex, u32 numBytes)
{
	memcpy(outBuffer, mData + startIndex, numBytes);
}// END GetBytes




// searches for the passed string of bytes, if successful returns the start index of the bytes otherwise -1
s32 cByteStream::Find(const char* bytes, u32 numBytes) const
{
	// slow and uses memory !
	std::string str(reinterpret_cast<const char *> (mData));

	u32 index = static_cast<u32> (str.find(bytes, 0, numBytes));

	if(index == std::string::npos)
	{
		return -1;
	}
	return index;
}// END Find




void cByteStream::Expand(u32 capcacity)
{
	assert(capcacity >= Size());

	mCapcacity=capcacity;

	u8* newData = new u8[capcacity];
	memset(newData, 0, mCapcacity);

	if(mData)
	{
		memcpy(newData, mData, mSize);
		delete[] mData;		
	}

	mData = newData;
}// END Expand