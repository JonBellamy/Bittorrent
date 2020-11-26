// Jon Bellamy



#if USE_PCH
#include "stdafx.h"
#endif


#include "SegmentBuffer.h"


#include <assert.h>
#include <algorithm>



namespace net {




cSegmentBuffer::cSegmentBuffer(u32 maxSize)
: MAX_SIZE(maxSize)
{
}// END cSegmentBuffer



cSegmentBuffer::~cSegmentBuffer()
{
}// END ~cSegmentBuffer



bool cSegmentBuffer::AddSegment(u32 time, const cReliableUdpPacket& segment)
{
	if(MAX_SIZE > 0 &&
		SizeInBytes() + segment.PacketSize() + sizeof(sPacketMetaData) > MAX_SIZE)
	{
		printf("***** WARNING segment buffer full ***** \n");
		assert(0);
		return false;
	}

	sPacketMetaData meta;
	meta.mTime = time;
	meta.mRetransmitCount = 0;
	meta.mEakCount = 0;

	// we already have this segment don't add it again
	cPacketEqual f(segment.SequenceNumber());
	if(find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f) != mSegmentBuffer.end())
	{
		return false;
	}

	mSegmentBuffer.push_back(cSegmentBufferItem(meta, segment));
	return true;
}// END AddSegment



bool cSegmentBuffer::RemoveSegment(u16 sequenceNumber)
{
	cPacketEqual f(sequenceNumber);
	SegmentBufIterator iter = find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f);
	if (iter != mSegmentBuffer.end())
	{
		mSegmentBuffer.erase(iter);

		// 2 segments with same sequence number. yikes
		assert(find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f) == mSegmentBuffer.end());

		return true;
	}
	return false;
}// END RemoveSegment



// we count on the fact that we push segments onto the end of the list in the order we send them.
// this means that the order cannot be disturbed despite ACKs & EAKs
u32 cSegmentBuffer::RemoveAllSegmentsSentBefore(u16 sequenceNumber)
{
	cPacketEqual f(sequenceNumber);
	SegmentBufIterator iter = find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f);
	if (iter == mSegmentBuffer.end())
	{
		return 0;
	}

	u32 count = static_cast<u32> (mSegmentBuffer.size());

	//printf("1 mSegmentBuffer size %d\n", mSegmentBuffer.size());

	iter++;
	mSegmentBuffer.erase(mSegmentBuffer.begin(), iter);

	//printf("2 mSegmentBuffer size %d\n", mSegmentBuffer.size());

	count = count - mSegmentBuffer.size();

	/*
	u32 count=1;
	// found, pop all front elements until our sequence number is at the front, then pop it off as well
	while(mSegmentBuffer.size() > 0 && iter != mSegmentBuffer.begin())
	{
		mSegmentBuffer.pop_front();
		count++;
	}
	assert(mSegmentBuffer.size() > 0);
	printf("count %d\n", count);
	mSegmentBuffer.pop_front();

	printf("2 mSegmentBuffer size %d\n", mSegmentBuffer.size());
	*/

	// 2 segments with same sequence number. yikes
	assert(find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f) == mSegmentBuffer.end());
	return count;
}// END RemoveAllSegmentsSentBefore



bool cSegmentBuffer::PopFront(cReliableUdpPacket* pSegmentOut)
{
	if(NumElements() == 0)
	{
		return false;
	}

	*pSegmentOut = mSegmentBuffer[0].mSegment;
	mSegmentBuffer.pop_front();
	return true;
}// END PopFront



bool cSegmentBuffer::ContainsSegment(u16 sequenceNumber) const
{
	cPacketEqual f(sequenceNumber);
	ConstSegmentBufIterator iter = find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f);
	return(iter != mSegmentBuffer.end());
}// END ContainsSegment



// this item becomes invalid if you call any other cSegmentBuffer functions
cSegmentBuffer::cSegmentBufferItem* cSegmentBuffer::RetrieveSegment(u16 sequenceNumber)
{
	cPacketEqual f(sequenceNumber);
	SegmentBufIterator iter = find_if(mSegmentBuffer.begin(), mSegmentBuffer.end(), f);
	if (iter == mSegmentBuffer.end())
	{
		return NULL;
	}
	cSegmentBufferItem& seg = *iter;
	return &seg;
}// END RetrieveSegment



// this item becomes invalid if you call any other cSegmentBuffer functions
cSegmentBuffer::cSegmentBufferItem* cSegmentBuffer::RetrieveSegmentAtIndex(u32 index)
{
	if(index >= NumElements())
	{
		return NULL;
	}
	return &mSegmentBuffer[index];
}// END RetrieveSegmentAtIndex



u32 cSegmentBuffer::SizeInBytes() const
{
	u32 count=0;
	for(ConstSegmentBufIterator iter = mSegmentBuffer.begin(); iter != mSegmentBuffer.end(); iter++)
	{
		count += (*iter).mSegment.PacketSize();
	}
	return count;
}// END SizeInBytes




} //namespace net

