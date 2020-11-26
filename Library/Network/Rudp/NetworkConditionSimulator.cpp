// Jon Bellamy



#if USE_PCH
#include "stdafx.h"
#endif

#include "NetworkConditionSimulator.h"


#if SIMULATE_NETWORK_CONDITIONS



#include "General/Rand.h"



namespace net {




cNetworkConditionSimulator::cNetworkConditionSimulator()
: mTimer(cTimer::STOPWATCH)
, mSocket(NULL)
, mLastLatencyChangeTime(0)
{
	mTimer.Start();

	mLatency.Submit(RandRange(MIN_LATENCY_MS, MAX_LATENCY_MS));
}// END cNetworkConditionSimulator



cNetworkConditionSimulator::~cNetworkConditionSimulator()
{
}// END ~cNetworkConditionSimulator



void cNetworkConditionSimulator::AddSegment(const cReliableUdpPacket& segment)
{
	assert(mSocket);

	static u32 xx=0;
	u32 rand = Rand32(100);
	xx++;
	if(rand < LOST_PACKET_PERCENTAGE)
	{
		return;
	}

	if(segment.IsFlagSet(ACK))
	{
		assert(segment.Header().ackNumber >= mSocket->mLastAckNumberSent);
	}

	mSegmentBuffer.AddSegment(mTimer.ElpasedMs(), segment);


	// out of order packets
	if( mSegmentBuffer.NumElements() >= 2 &&
		Rand32(100) < OUT_OF_ORDER_PERCENTAGE)
	{
		cSegmentBuffer::cSegmentBufferItem* p1 = mSegmentBuffer.RetrieveSegmentAtIndex(0);
		cSegmentBuffer::cSegmentBufferItem* p2 = mSegmentBuffer.RetrieveSegmentAtIndex(1);
		assert(p1 && p2);
		if(p1 && p2)
		{
			cSegmentBuffer::cSegmentBufferItem tmp(*p1);
			*p1 = *p2;
			*p2 = tmp;

			u32 tmpTime = p1->mPacketMetaData.mTime;
			p1->mPacketMetaData.mTime = p2->mPacketMetaData.mTime;
			p2->mPacketMetaData.mTime = tmpTime;
		}
	}

	SanityCheck();
}// END AddSegment



void cNetworkConditionSimulator::Process()
{
	if(!mSocket)
	{
		return;
	}

	mTimer.Process();


	// alter the latency a little every second
	if(mLastLatencyChangeTime != mTimer.ElpasedMs() &&
	   mTimer.ElpasedMs() % 3000 == 0)
	{		
		mLastLatencyChangeTime = mTimer.ElpasedMs();

		s32 latencyChange = -LATENCY_CHANGE_RANGE;
		latencyChange += Rand32(LATENCY_CHANGE_RANGE*2);

		// swing up to 2Ms either way
//		s32 latencyChange = -2;
//		latencyChange += Rand32(4);

		u32 latency = mLatency.Average() + latencyChange;
		if(latency < MIN_LATENCY_MS)
		{
			latency = MIN_LATENCY_MS;
		}
		if(latency > MAX_LATENCY_MS)
		{
			latency = MAX_LATENCY_MS;
		}
		mLatency.Submit(latency);

		//printf("SIMULATED LATENCY t=%d new latency= %d\n", mTimer.ElpasedMs(), mLatency.Average());
	}

	if(mSegmentBuffer.NumElements() == 0)
	{
		return;
	}

	
	SanityCheck();

	// send as many packets as the rand latency timer allows
	bool bSent;
	do
	{
		bSent = false;
		cSegmentBuffer::cSegmentBufferItem* pItem = mSegmentBuffer.RetrieveSegmentAtIndex(0);

		if(pItem && mTimer.ElpasedMs() - pItem->mPacketMetaData.mTime >= mLatency.Average())
		{
			cReliableUdpPacket packet;
			bool ret = mSegmentBuffer.PopFront(&packet);
			if(ret)
			{			
				//printf("Delayed send seq %d\n", packet.SequenceNumber());
				mSocket->cUdpSocket::Send(const_cast<cReliableUdpPacket&>(packet).Packet(), packet.PacketSize());
				bSent=true;
			}
		}
	}
	while(bSent);
}// END Process



void cNetworkConditionSimulator::SanityCheck()
{
	if(mSegmentBuffer.NumElements() < 2)
	{
		return;
	}

	for(u32 i=0; i < mSegmentBuffer.NumElements()-1; i++)
	{
		cSegmentBuffer::cSegmentBufferItem* pItem1 = mSegmentBuffer.RetrieveSegmentAtIndex(i);
		cSegmentBuffer::cSegmentBufferItem* pItem2 = mSegmentBuffer.RetrieveSegmentAtIndex(i+1);

		// what is about to be sent 
		if(pItem1->mSegment.IsFlagSet(ACK) && pItem2->mSegment.IsFlagSet(ACK))
		{
			//assert(pItem1->mSegment.Header().ackNumber <= pItem2->mSegment.Header().ackNumber);

			//assert(pItem1->mPacketMetaData.mTime <= pItem2->mPacketMetaData.mTime);
		}
	}
}// END SanityCheck



void cNetworkConditionSimulator::SetSocket(cReliableUdpSocket* socket)
{ 
	mSocket = socket; 
}


} //namespace net


#endif // SIMULATE_NETWORK_CONDITIONS
