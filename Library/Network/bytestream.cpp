
#include "ByteStream.h"

#include <memory.h>


cByteStream::cByteStream(u32 capcacity, bool canExpand)
: mCapcacity(0)
, mSize(0)
, mData(NULL) 
, mCanExpand(true)
{
	Expand(capcacity);
	mCanExpand = canExpand;
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
		mData=NULL;
	}
}// END ~cByteStream



const cByteStream& cByteStream::operator= (const cByteStream& rhs)
{
	mCanExpand = true;

	Expand(rhs.Capacity());

	if(rhs.Capacity() > 0)
	{
		memcpy(mData, rhs.mData, rhs.Size());
	}
	mSize = rhs.Size();
	mCanExpand = rhs.mCanExpand;
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
	assert(mSize <= Capacity());
	return mData[index];
}// END operator[]



const u8& cByteStream::operator[] (u32 index) const
{
	assert(index < Size());
	return mData[index];
}// END operator[]



void cByteStream::Clear(bool zeroMemory)
{
	if(mData && zeroMemory)
	{
		memset(mData, 0, mCapcacity);
	}
	mSize = 0;
}// END Clear



void cByteStream::ClearAndResize(u32 newCapacity, bool resizeToNextPowerOfTwo)
{
	Clear();
	Expand(newCapacity, resizeToNextPowerOfTwo);
}// END ClearAndResize



void cByteStream::ZeroUnused()
{
	if(mData)
	{
		memset(mData+mSize, 0, mCapcacity - mSize);
	}
}// END ZeroUnused



// removes all free space from the array
void cByteStream::Trim()
{
	Expand(Size());
}// END Trim



bool cByteStream::StreamBytes(const u8* newData, u32 numBytes)
{
	if(Capacity()==0)
	{
		Expand(DEFAULT_START_CAPACITY);
	}

	// double the size if needed
	if(mSize + numBytes > mCapcacity)
	{
		if(CanExpand() == false)
		{
			assert(0);
			return false;
		}

		u32 newCapacity = Capacity() * 2;
		while(newCapacity < (Size() + numBytes))
		{
			newCapacity *= 2;
		}
		
		Expand(newCapacity);
	}

	assert(mData);

	memcpy(mData + mSize, newData, numBytes);
	mSize += numBytes;

	assert(mSize <= mCapcacity);

	return true;
}// END StreamBytes



// the byte at startIndex is removed
void cByteStream::RemoveBytes(u32 startIndex, u32 numBytes)
{
	assert(Size() >= (startIndex + numBytes));
	assert(mData);

	memmove_s(mData + startIndex, Capacity()-startIndex, mData + startIndex + numBytes, Size()-numBytes);
	mSize -= numBytes;

	/*
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
	*/
}// END RemoveBytes



void cByteStream::GetBytes(u8* outBuffer, u32 startIndex, u32 numBytes)
{
	assert(mData && Size() > (startIndex + numBytes));
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



u8 cByteStream::ReadU8(u32 index)
{
	assert(Size() >= index);
	return (u8) mData[index];
}// END ReadU8



u16 cByteStream::ReadU16(u32 index)
{
	assert(Size() >= (index + sizeof(u16)));
	u16 ret;
	memcpy(&ret, &mData[index], sizeof(u16));
	return ret;
}// END ReadU16



u32 cByteStream::ReadU32(u32 index)
{
	assert(Size() >= (index + sizeof(u32)));
	u32 ret;
	memcpy(&ret, &mData[index], sizeof(u32));
	return ret;
}// END ReadU32



void cByteStream::Expand(u32 capcacity, bool expandToNextPowerOfTwo)
{
	assert(capcacity >= Size());

	if(CanExpand() == false)
	{
		assert(0);
		return;
	}

	if(expandToNextPowerOfTwo)
	{
		capcacity = NextPowerOfTwo(capcacity);
	}

	mCapcacity=capcacity;

	if(mCapcacity==0 && mData)
	{
		delete[] mData;
		mData=NULL;
		return;
	}

	u8* newData = new u8[capcacity];
	assert(newData);
	memset(newData, 0, mCapcacity);

	if(mData)
	{
		memcpy(newData, mData, mSize);
		delete[] mData;		
	}

	mData = newData;
}// END Expand



// used to expand the 
u32 cByteStream::NextPowerOfTwo(u32 val) const
{
	val--;
	val = (val >> 1) | val;
	val = (val >> 2) | val;
	val = (val >> 4) | val;
	val = (val >> 8) | val;
	val = (val >> 16) | val;
	val++; // Val is now the next highest power of 2.
	return val;
}// END NextPowerOfTwo