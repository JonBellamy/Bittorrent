// Jon Bellamy


#ifndef RUDP_SEGBUF_H
#define RUDP_SEGBUF_H


#include <deque>

#include "ReliableUdpPacket.h"


namespace net {


class cSegmentBuffer
{
public:
    cSegmentBuffer(u32 maxSize=0);
	~cSegmentBuffer();

	// TODO : the meta data isn't used for input queue, seems silly
	bool AddSegment(u32 time, const cReliableUdpPacket& segment);

	bool RemoveSegment(u16 sequenceNumber);
	u32 RemoveAllSegmentsSentBefore(u16 sequenceNumber);

	bool PopFront(cReliableUdpPacket* pSegmentOut);

	typedef struct
	{
		u32 mTime;
		u8 mRetransmitCount;
		u8 mEakCount;				// number of times we have sent an eak for this segment
	}sPacketMetaData;


	// all this class does is append the above meta data to the beginning of the packet. i am concerned that this is 
	// yet another copy of every bit of data we send
	class cSegmentBufferItem
	{
	public:
		cSegmentBufferItem(const sPacketMetaData& meta, const cReliableUdpPacket& segment)
		{
			mPacketMetaData = meta;
			mSegment = segment;
		}

		cSegmentBufferItem(const cSegmentBufferItem& rhs)
		{
			*this = rhs;
		}

		const cSegmentBufferItem& operator= (const cSegmentBufferItem& rhs)
		{
			mPacketMetaData = rhs.mPacketMetaData;
			mSegment = rhs.mSegment;
			return *this;
		}

		bool operator== (const cSegmentBufferItem& rhs)
		{
			return mSegment.SequenceNumber() == rhs.mSegment.SequenceNumber();
		}

		sPacketMetaData mPacketMetaData;
		cReliableUdpPacket mSegment;
	};


	bool ContainsSegment(u16 sequenceNumber) const;

	// this item becomes invalid if you call any other cSegmentBuffer functions
	cSegmentBufferItem* RetrieveSegment(u16 sequenceNumber);

	// this item becomes invalid if you call any other cSegmentBuffer functions
	cSegmentBufferItem* RetrieveSegmentAtIndex(u32 index);

	void Clear();

	u32 NumElements() const;

	u32 SizeInBytes() const;

	u32 FreeSpace() const { return MAX_SIZE - SizeInBytes(); }

private:

	typedef std::deque<cSegmentBufferItem>::iterator SegmentBufIterator;
	typedef std::deque<cSegmentBufferItem>::const_iterator ConstSegmentBufIterator;
	
	std::deque<cSegmentBufferItem> mSegmentBuffer;

	const u32 MAX_SIZE;					// the buffer will not grow beyoned this size (in bytes). 0 == no limit



	// here are our search functors
	class cPacketEqual
	{
	public:

		cPacketEqual( u32 sequenceNumber ) { mSequenceNumber = sequenceNumber; }
		bool operator() ( const cSegmentBufferItem& rhs ) { return rhs.mSegment.SequenceNumber() == mSequenceNumber; }

	private:
		u32 mSequenceNumber;
	};



	class cPacketLtEqual
	{
	public:

		cPacketLtEqual( u32 sequenceNumber ) { mSequenceNumber = sequenceNumber; }
		bool operator() ( const cSegmentBufferItem& rhs ) { return rhs.mSegment.SequenceNumber() <= mSequenceNumber; }

	private:
		u32 mSequenceNumber;
	};
};





//////////////////////////////////////////////////////////////////////////
// inlines


inline void cSegmentBuffer::Clear()
{
	mSegmentBuffer.clear();
}// END Clear



inline u32 cSegmentBuffer::NumElements() const
{
	return static_cast<u32>(mSegmentBuffer.size());
}// END NumElements







} // namespace net



#endif // RUDP_SEGBUF_H
