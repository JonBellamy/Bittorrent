// Jon Bellamy


#ifndef NET_COND_H
#define NET_COND_H

#include "Network/NetSettings.h"

#if SIMULATE_NETWORK_CONDITIONS

#include "ReliableUdpSocket.h"
#include "ReliableUdpPacket.h"
#include "SegmentBuffer.h"
#include "General/Timer.h"
#include "Math/RollingAverage.h"

namespace net {



// inherit from segment buffer instead?
class cNetworkConditionSimulator
{
public:
    cNetworkConditionSimulator();
	~cNetworkConditionSimulator();

	void SetSocket(cReliableUdpSocket* socket);

	void Process();

	void AddSegment(const cReliableUdpPacket& segment);

	void SanityCheck();

private:

	enum
	{
		LOST_PACKET_PERCENTAGE = 1,
		OUT_OF_ORDER_PERCENTAGE = 3,
	
		// remember that the rtt is double this so an internet ping of 500 would be 250 here
		MIN_LATENCY_MS = 50,
		MAX_LATENCY_MS = 300,

		// big changes in latency obviously results in many retransmits
		LATENCY_CHANGE_RANGE = 100,
	};

	cSegmentBuffer mSegmentBuffer;
	cReliableUdpSocket* mSocket;
	cTimer mTimer;

	// a coarse grained average gives us more pronounced latency changes (which we generally want)
	cRollingAverage<u32, 4> mLatency;
	u32 mLastLatencyChangeTime;
};









} // namespace net


#endif // SIMULATE_NETWORK_CONDITIONS


#endif // NET_COND_H
