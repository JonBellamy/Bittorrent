// Jon Bellamy
// A reliable udp socket supports:
// Reliable transport, in order transport, flow control

// it does NOT support:
// slow start, congestion control



#ifndef RUDP_NET_SOCKET_H
#define RUDP_NET_SOCKET_H


#include <string>

#include "Network/NetSettings.h"
#include "Network/UdpSocket.h"
#include "Network/Rudp/ReliableUdpPacket.h"
#include "Network/Rudp/SegmentBuffer.h"
#include "General/Timer.h"
#include "Math/RollingAverage.h"

#if RUDP_PACKET_DEBUGGING
#include "Network/Rudp/HtmlPacketLog.h"
#endif


namespace net {





class cReliableUdpSocket : private cUdpSocket
{
public:
    cReliableUdpSocket();
	~cReliableUdpSocket();

	bool Open(const cSockAddr& localAddrToBindTo);
	void Close();

    bool Connect(const cSockAddr& addrToConnectTo);

	// TODO : callback function : bool AcceptConnnectionFrom(const cSockAddr from&) ??
	// keep calling this to listen for incoming connections, returns true when we get one
	bool Listen();

	// Send & Recv match the standard udp interface
	s32 Recv(void* pBuffer, u32 bufSize);
	s32 Send(const void* pBuffer, u32 bufSize);

	void Process();

	typedef void (*ConnectionClosedCb) (void *);
	void SetConnectionClosedCallback(ConnectionClosedCb cb, void* pParam=NULL);


	////////////////////////////////////////////////////////////////////
	// call through's

	bool IsOpen() const;


	////////////////////////////////////////////////////////////////////
	// inlines

public:
	bool ConnectionEstablished() const;
	const cSockAddr& ConnectedPeerAddress() const;
	bool IsFlowControlActive() const;
	bool IsConnectionClosing() const;

private:
	u32 NumSegmentsCanSendBeforeFlowControlStarts() const;
	bool PingSegmentOutstanding() const;
	

private:

	enum
	{
		SEQ_IGNORE = 0xffff,
		MAX_OUTSTANDING_SEGMENTS = 16,										// how many segments we will send without getting an ack. if we go over, new data is added to mOutputQueue
		MAX_DATAGRAMS_ON_INPUT_QUEUE = MAX_OUTSTANDING_SEGMENTS,

		DEFAULT_TIMEWAIT_LENGTH_MS = 10000,									// after sending and receiving a FIN we wait 10 seconds to check our final ack gets there then we close the socket

		PING_FREQUENCY_MS = 2000,
		
		SYN_RETRANSMIT_PERIOD_MS = 5000,									// SYN segment retransmit period (for new connections)
		DEFAULT_RETRANSMIT_PERIOD_MS = 600,									// how long we wait without an ACK before resending the packet
		INITIAL_RETRANSMIT_PERIDO_MS = 1200,								// before we have built a ping history use this value as our start retransmit timer		
		DEFAULT_MAX_RETRANSMISSIONS = 4,
		DEFAULT_MAX_EAK = 1,												// how many eaks will we send for a segment before we just rely on the retransmission timer

		DEFAULT_OUT_OF_SEQUENCE_ACK_COUNTER = 3,							// how many out of sequence packets do we get before sending an EAK
		DEFAULT_CUMULATIVE_ACK_COUNTER = 3,									// how many unAckd segments we will receive before we force send a standalone ack
		DEFAULT_CUMULATIVE_ACK_TIMER_MS = 300,								// how long we will wait without a piggyback ack being sent before we force send a standalone ack		
		DEFAULT_NUL_SEGMENT_TIMER_MS = 2000									// how long without sending a segment before we send a keep alive
	};

	enum
	{
		RECV_BUFFER_SIZE = 1024 * 64,
		OUTPUT_QUEUE_SIZE = 128 * 1024
	};

	typedef enum
	{
		DATA_SENT = 0,
		DATA_QUEUED,
		SEND_FAILED_WINSOCK_ERROR
	}RudpSendResult;

	typedef enum
	{
		CLOSED=0,
		LISTEN,
		SYN_SENT,
		SYN_RECEIVED,
		ESTABLISHED,
		CLOSE_WAIT,
		LAST_ACK,
		FIN_WAIT_1,
		FIN_WAIT_2,
		TIME_WAIT,		
	}SocketState;


	u16 LastInOrderPacketNumberReceived() const;

	// closes the udp socket, no questions asked
	void CloseSocket();

	// gets the next pending packet from the datagram socket, nothing more
	bool RecvRudpPacket(cReliableUdpPacket* pPacketOut, cSockAddr* fromOut=NULL);

	// pumps the packet through the socket, nothing more
	s32 SendRudpPacket(cReliableUdpPacket& p, bool isRetransmission=false);

	// receive up to MAX_DATAGRAMS_ON_INPUT_QUEUE pending segments from udp, send any ack's we need to
	u32 ServiceInputQueue();

	// if flow control has kicked in and we have started queuing up packets to send, they get priority above everything else
	void ServiceOutputQueue();

	// check if the next packet(s) we are expecting are pending on the out of order queue
	void ServiceOutOfOrderQueue();

	// returns true if the packet was added to the input queue
	bool OnInOrderPacketReceived(const cReliableUdpPacket& packet);

	// TODO : #if the bool properly
	// last stop before the packet is sent, sets the ack flag and potentially queues up the data if flow control is active
	RudpSendResult Send(cReliableUdpPacket& p, s32* pBytesSentOut=NULL);


	bool NextPendingPacketOnInputQueue(cReliableUdpPacket* pPacketOut);
	bool NextPendingPacketOnOutputQueue(cReliableUdpPacket* pPacketOut);

	void ProcessRetransmit();

	u16 IncrementSequenceNumber();

	bool IsDupePacket(const cReliableUdpPacket& p) const;
	bool IsOutOfOrderPacket(const cReliableUdpPacket& p) const;

	bool AreSequenceNumbersInValidRange(u16 seq1, u16 seq2) const;
	u16 AbsDifferenceBetweenSequenceNumbers(u16 seq1, u16 seq2) const;
	u16 AcceptableSequenceNumberWindow() const;


	//////////////////////////////////////////////////////////////////////////
	// Control flag handlers
	
	bool NeedToPing() const;
	bool AddPingToSegment(cReliableUdpPacket& segment);
	void OnPngSegmentAck();

	// sends a standalone ack segment that is not piggybacked on data
	void SendAckSegment();
	void SendEakSegment();
	
	void SendRstSegment();
	void SendFinSegment();
	
	bool SendNulSegment();

	void OnACKReceived(const cReliableUdpPacket& segment);
	void OnEAKReceived(const cReliableUdpPacket& segment);
	void OnSYNReceived(const cReliableUdpPacket& synSegment);
	void OnFINReceived();
	void OnRSTReceived();
	void OnNULReceived();
	//////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////
	// States 
	std::string StateAsString(SocketState state) const;
	void State(SocketState state);

	void ProcessState_SynSent();
	void ProcessState_SynReceived();
	void ProcessState_Established();
	void ProcessState_CloseWait();
	void ProcessState_LastAck();
	void ProcessState_FinWait1();
	void ProcessState_TimeWait();
	//////////////////////////////////////////////////////////////////////////




	//////////////////////////////////////////////////////////////////////////
	// data

	SocketState mSocketState;
	cSockAddr mConnectedAddress;
	mutable cTimer mTimer;

	cSegmentBuffer mInputQueue;					// when the connection is established we receive packets as soon as they arrive and store them here so we can pick out control messages such as ack segments
	cSegmentBuffer mOutputQueue;				// when flow control is active this buffer will fill with packets waiting to be sent
	cSegmentBuffer mOutOfOrderSegmentBuffer;	// segments we have received that are valid but out of order
	cSegmentBuffer mUnAckdSegmentBuffer;		// segments we have sent but have yet to be ack'd by the other side
	u8 mRecvBuffer[RECV_BUFFER_SIZE];			// we need to accommodate the largest possible datagram size

	u16 mSequenceNumber;						// our sequence number
	u16 mLastSequenceNumberSentThroughSocket;	// the last seq number that went through, not queued or any other gubbins, the last one on the wire
	
	u16 mNextExpectedSequenceNumber;			// the next in order packet received should have this sequence number
	
	u16	mLastAckNumberSent;						// what was the sequence number we last ack'd
	u32 mOutOfSequenceEakCounter;				// how many out of sequence packets do we receive before sending an EAK
	
	u32 mLastAckSendTime;						// when did we last ack a packet from the other side?
	u32 mLastSegmentSendTime;					// when did we last send anything? Used to send keep alive messages when required
	u32 mTimeEnteredTimeWaitState;				// used to wait for lost ack's when closing the conection
	
	bool mPingOutstanding;
	u32 mLastPingSendTime;						// when did we send out the ping segment
	u16 mPingSegmentSeqNumber;					// the segment we are timing
	cRollingAverage<u32, 4> mConnectionPing;	// we ping the otherside every PING_FREQUENCY_MS and store (in here) the RTT is on that segment
	

	ConnectionClosedCb mConnectionClosedCb;
	void* mConnectionLostParam;


	//////////////////////////////////////////////////////////////////////////
	// socket parameters

	//const u32 RETRANSMISSION_PERIOD;			// how long we wait without an ACK before we resend
	const u32 MAX_RETRANSMISSIONS;				// how many times will we resend a segment before we consider the connection broken
	const u16 CUMULATIVE_ACK_COUNTER; 			// how many unAckd segments we will receive before we force send a standalone ack
	const u32 CUMULATIVE_ACK_TIMER_MS; 			// how long we will wait without a piggyback ack being sent before we force send a standalone ack


	//////////////////////////////////////////////////////////////////////////
	// stats

	u32 mPacketsSent;
	u32 mPacketsReceived;
	u32 mPacketsRetransmitted;
	u32 mOutOfOrderPacketsReceived;
	u32 mCorruptPacketsReceived;
	u32 mHighestLatency;
	u32 mLowestLatency;
//	u32 mBytesSent;
//	u32 mBytesReceived;
	u32 mUserDatagramsSent;
	u32 mUserDatagramsReceived;


#if RUDP_PACKET_DEBUGGING
	friend class cHtmlPacketLog;
	cHtmlPacketLog mPacketLog;
#endif
	

#if SIMULATE_NETWORK_CONDITIONS
	friend class cNetworkConditionSimulator;
#endif
};


// send return codes, our error codes come after the winsock error codes
enum
{
	OUTPUT_QUEUE_FULL = SOCKET_ERROR - 1
};


////////////////////////////////////////////////////////////////////
// inlines



inline bool cReliableUdpSocket::IsOpen() const
{
	return cUdpSocket::IsOpen();
}// END IsOpen



inline bool cReliableUdpSocket::ConnectionEstablished() const
{
	return mSocketState == ESTABLISHED;
}// END ConnectionEstablished



inline bool cReliableUdpSocket::IsConnectionClosing() const
{
	return (mSocketState == CLOSE_WAIT || mSocketState == LAST_ACK || mSocketState == FIN_WAIT_1 || mSocketState == FIN_WAIT_2 || mSocketState == TIME_WAIT);
}// END IsConnectionClosing



inline const cSockAddr& cReliableUdpSocket::ConnectedPeerAddress() const
{
	return mConnectedAddress;
}// END ConnectedPeerAddress



inline bool cReliableUdpSocket::IsFlowControlActive() const
{
	return mUnAckdSegmentBuffer.NumElements() >= MAX_OUTSTANDING_SEGMENTS;
}// END IsFlowControlActive



inline u32 cReliableUdpSocket::NumSegmentsCanSendBeforeFlowControlStarts() const
{
	return MAX_OUTSTANDING_SEGMENTS - mUnAckdSegmentBuffer.NumElements();
}// END NumSegmentsCanSendBeforeFlowControlStarts



inline bool cReliableUdpSocket::PingSegmentOutstanding() const
{
	return mPingOutstanding;
}// END PingSegmentOutstanding



inline void cReliableUdpSocket::SetConnectionClosedCallback(ConnectionClosedCb cb, void* pParam)
{
	mConnectionClosedCb = cb;
	mConnectionLostParam = pParam;
}// END SetConnectionClosedCallback


//////////////////////////////////////////////////////////////////////////
// Control segment layout

// this gets sent with a SYN message to setup various options
typedef struct  
{
	u8 mProtocolVer : 4;
	u8 mReserved : 4;
	u8 mMaxOutstandingSegments;
	u16 mMaxSegmentSize;
	u16 mRetransmissionTimeoutMs;
	u16 mCumulativeAckTimeout;
	u16 mNullSegmentTimeoutMs;
	u8 mMaxRetransmissions;
	u8 mMaxCumulativeAck;
	u8 mMaxOutOfSequence;
	u32 mConnectionIdentifier;
}SynSegmentPayload;


} // namespace net



#endif // RUDP_NET_SOCKET_H
