// Jon Bellamy
// A reliable udp socket supports:
// Reliable transport, in order transport, flow control

// it does NOT support:
// slow start, congestion control



#if USE_PCH
#include "stdafx.h"
#endif

#include "ReliableUdpSocket.h"


#include <assert.h>

#include "Network/NetSettings.h"
#include "General/Rand.h"


#if SIMULATE_NETWORK_CONDITIONS
#include "Network/Rudp/NetworkConditionSimulator.h"
net::cNetworkConditionSimulator gNetConditionsSimulator;
#endif
//#define NETSIM_ONLY(x) #if SIMULATE_NETWORK_CONDITIONS (x) #endif


// infinite retries, wait forever for ack's etc
#define RESILIENT_CONNECTIONS 0


namespace net {




cReliableUdpSocket::cReliableUdpSocket()
: mSocketState(CLOSED)
, mSequenceNumber(0)
, mNextExpectedSequenceNumber(SEQ_IGNORE)
, mLastAckNumberSent(0)
, mLastSequenceNumberSentThroughSocket(0)
, mConnectedAddress()
, mTimer(cTimer::STOPWATCH)	
, mInputQueue(0)
, mOutputQueue(OUTPUT_QUEUE_SIZE)
, mOutOfOrderSegmentBuffer(0)
, mUnAckdSegmentBuffer(0)
, mLastAckSendTime(0)
, mLastSegmentSendTime(0)
, mOutOfSequenceEakCounter(0)
, mTimeEnteredTimeWaitState(0)
, mPacketsSent(0)
, mPacketsReceived(0)
, mPacketsRetransmitted(0)
, mOutOfOrderPacketsReceived(0)
, mCorruptPacketsReceived(0)
, mHighestLatency(0)
, mLowestLatency(0xFFFFFFFF)
, mUserDatagramsSent(0)
, mUserDatagramsReceived(0)
, mPingOutstanding(false)
, mLastPingSendTime(0)
, mPingSegmentSeqNumber(0)	
, mConnectionClosedCb(NULL)
, mConnectionLostParam(NULL)
//, RETRANSMISSION_PERIOD(DEFAULT_RETRANSMIT_PERIOD_MS)
, MAX_RETRANSMISSIONS(DEFAULT_MAX_RETRANSMISSIONS)
, CUMULATIVE_ACK_COUNTER(DEFAULT_CUMULATIVE_ACK_COUNTER)
, CUMULATIVE_ACK_TIMER_MS(DEFAULT_CUMULATIVE_ACK_TIMER_MS)
{
#if RESILIENT_CONNECTIONS
	const_cast<u32>(MAX_RETRANSMISSIONS) = 0;
#endif

#if SIMULATE_NETWORK_CONDITIONS
	gNetConditionsSimulator.SetSocket(this);
#endif

#if RUDP_PACKET_DEBUGGING
	mPacketLog.SetSocket(this);
#endif

	mTimer.Start();
	mTimer.Process();
}// END cReliableUdpSocket



cReliableUdpSocket::~cReliableUdpSocket()
{
	CloseSocket();
	State(CLOSED);
}// END ~cReliableUdpSocket



bool cReliableUdpSocket::Open(const cSockAddr& localAddrToBindTo)
{
	mInputQueue.Clear();
	mOutputQueue.Clear();
	mOutOfOrderSegmentBuffer.Clear();
	mUnAckdSegmentBuffer.Clear();
	memset(mRecvBuffer, 0, sizeof(mRecvBuffer));
		
	mSequenceNumber = SEQ_IGNORE;
	mNextExpectedSequenceNumber = SEQ_IGNORE;	
	mLastAckNumberSent = SEQ_IGNORE;
	mLastSequenceNumberSentThroughSocket = 0;
	mLastAckSendTime = 0;
	mLastSegmentSendTime = 0;
	mOutOfSequenceEakCounter = 0;
	mTimeEnteredTimeWaitState = 0;
	mPacketsSent = 0;
	mPacketsReceived = 0;
	mPacketsRetransmitted = 0;
	mOutOfOrderPacketsReceived = 0;
	mCorruptPacketsReceived = 0;
	mHighestLatency = 0;
	mLowestLatency = 0xFFFFFFFF;
	mUserDatagramsSent = 0;
	mUserDatagramsReceived = 0;
	mPingOutstanding = false;
	mLastPingSendTime = 0;
	mPingSegmentSeqNumber = 0;
	mConnectionPing.Clear();
	mConnectionPing.Submit(INITIAL_RETRANSMIT_PERIDO_MS);

#if RUDP_PACKET_DEBUGGING
	mPacketLog.Open("PacketLog.html");
#endif

	return cUdpSocket::Open(localAddrToBindTo, false);
}// END Open



void cReliableUdpSocket::Close()
{
	// keep this?
	if(!IsOpen() || IsConnectionClosing())
	{
		return;
	}

	if(ConnectionEstablished())
	{
		SendFinSegment();
		State(FIN_WAIT_1);
	}
	else
	{
		CloseSocket();
	}
}// END Close



// closes the udp socket, no questions asked
void cReliableUdpSocket::CloseSocket()
{
	cUdpSocket::Close();
	State(CLOSED);

	if(mConnectionClosedCb)
	{
		mConnectionClosedCb(mConnectionLostParam);
	}

#if RUDP_PACKET_DEBUGGING
	mPacketLog.Close();
#endif
}// END CloseSocket



bool cReliableUdpSocket::Connect(const cSockAddr& addrToConnectTo)
{
	if(!IsOpen())
	{
		return false;
	}

	// we cannot use an rudp socket to send and recv to multiple peers, connect the socket to one peer
	if(cUdpSocket::Connect(addrToConnectTo) == false)
	{
		printf("cUdpSocket::Connect() FAILED\n");
		return false;
	}

	mConnectedAddress = addrToConnectTo;

	// standard 3 way handshake

	mSequenceNumber = Rand16();
	//mSequenceNumber = 65520;
	if(mSequenceNumber == SEQ_IGNORE)
	{
		mSequenceNumber++;
	}
	mNextExpectedSequenceNumber = SEQ_IGNORE;
	mLastAckSendTime = 0;
	mLastAckNumberSent = 0;

	SynSegmentPayload options;
	memset(&options, 0, sizeof(SynSegmentPayload));
	options.mProtocolVer = 1;
	// TODO ...
//	options.mMaxOutstandingSegments = ;
//	options.mMaxSegmentSize = ;
//	options.mRetransmissionTimeoutMs = ;
//	options.mCumulativeAckTimeout = ;
//	options.mNullSegmentTimeoutMs = ;
//	options.mMaxRetransmissions = ;
//	options.mMaxCumulativeAck = ;
//	options.mMaxOutOfSequence = ;
	options.mConnectionIdentifier = Rand32();


	cReliableUdpPacket packet(SYN, reinterpret_cast<u8*>(&options), sizeof(SynSegmentPayload));
	packet.Header().sequenceNumber = mSequenceNumber;
	packet.Header().ackNumber = SEQ_IGNORE;

	Send(packet);

	State(SYN_SENT);
	return true;
}// END Connect



// TODO : callback function : bool AcceptConnnectionFrom(const cSockAddr from&) ??
// TODO : what if 2 datagrams are pending on the socket, does the second get cleared when we Connect?? Gotcha right here !!!!!!!!!

// keep calling this to listen for incoming connections, returns true when we get one
bool cReliableUdpSocket::Listen()
{
	if(!IsOpen())
	{
		return false;
	}

	State(LISTEN);

	cReliableUdpPacket packet;
	cSockAddr from;

	
	if(RecvRudpPacket(&packet, &from) == false)
	{
		return false;
	}

	// we have a valid rudp packet, it has not been added to any queues

	if(packet.IsFlagSet(SYN))
	{
		// we cannot use an rudp socket to send and recv to multiple peers, connect the socket to one peer
		if(cUdpSocket::Connect(from) == false)
		{
			printf("cUdpSocket::Connect() FAILED\n");
			return false;
		}

		mConnectedAddress = from;

		// standard 3 way handshake
		printf("Received SYN from %s\n", from.Ip().AsString());

		OnSYNReceived(packet);

		// we only expect SYN to be set here		

		mSequenceNumber = Rand16();
		//mSequenceNumber = 7000;
		if(mSequenceNumber == SEQ_IGNORE)
		{
			mSequenceNumber++;
		}
		mLastAckSendTime = 0;
		mLastAckNumberSent = 0;
		

		SynSegmentPayload options;
		memset(&options, 0, sizeof(SynSegmentPayload));
		options.mProtocolVer = 1;
		// TODO ...
		//	options.mMaxOutstandingSegments = ;
		//	options.mMaxSegmentSize = ;
		//	options.mRetransmissionTimeoutMs = ;
		//	options.mCumulativeAckTimeout = ;
		//	options.mNullSegmentTimeoutMs = ;
		//	options.mMaxRetransmissions = ;
		//	options.mMaxCumulativeAck = ;
		//	options.mMaxOutOfSequence = ;
		options.mConnectionIdentifier = Rand32();


		// ACK their SYN and send our own
		cReliableUdpPacket reply(SYN|ACK, reinterpret_cast<u8*>(&options), sizeof(SynSegmentPayload));
		reply.Header().sequenceNumber = mSequenceNumber;
		reply.Header().ackNumber = packet.Header().sequenceNumber;

		Send(reply);

		State(SYN_RECEIVED);

		return true;
	}


	// send RST?
	SendRstSegment();

	return false;
}// END Listen



void cReliableUdpSocket::Process()
{
	mTimer.Process();

	if(mSocketState == CLOSED)
	{
		return;
	}

	// we service the queues in the standard established state and also when a connection is closing. No new data will be accepted for send while in any closing state so we 
	// keep the queues serviced in order to get the ack's through and to allow pending and retransmit data to go through
	if(ConnectionEstablished() || IsConnectionClosing())
	{
		// cumulative ack check
		if( mLastAckNumberSent != LastInOrderPacketNumberReceived() &&
			mTimer.ElpasedMs() - mLastAckSendTime >= CUMULATIVE_ACK_TIMER_MS)
		{
			SendAckSegment();
		}

		assert(mInputQueue.NumElements() <= MAX_DATAGRAMS_ON_INPUT_QUEUE);
		
		ServiceInputQueue();

		ServiceOutputQueue();
	}	

	ProcessRetransmit();

#if SIMULATE_NETWORK_CONDITIONS
	gNetConditionsSimulator.Process();
#endif

	switch(mSocketState)
	{
	case LISTEN:
		Listen();
		break;

	case SYN_SENT:
		ProcessState_SynSent();
		break;

	case SYN_RECEIVED:
		ProcessState_SynReceived();
		break;

	case ESTABLISHED:
		ProcessState_Established();
		break;

	case CLOSE_WAIT:
		ProcessState_CloseWait();
		break;

	case LAST_ACK:
		ProcessState_LastAck();
		break;

	case FIN_WAIT_1:
		ProcessState_FinWait1();
		break;

	// we sit here until we get the FIN from the otherside, the logic is handled in OnFINReceived
	case FIN_WAIT_2:
		break;

	case TIME_WAIT:
		ProcessState_TimeWait();
		break;

	default:
		break;
	}
}// END Process



s32 cReliableUdpSocket::Recv(void* pBuffer, u32 bufSize)
{
	// we only read from the input queue, never from the socket
	cReliableUdpPacket packet;
	if(!NextPendingPacketOnInputQueue(&packet))
	{
		return 0; 
	}

	// out buffer is not big enough to hold the received data
	if(bufSize < packet.PayloadSize())
	{
		// If the datagram or message is larger than the buffer specified, the buffer is filled with the first part of the datagram, and recv generates the error WSAEMSGSIZE. 
		// For unreliable protocols (for example, UDP) the excess data is lost
		// bearing in mind this packet has already been ack'd if we get here the data is lost but its the applications fault
		assert(0);
		return WSAEMSGSIZE;
	}


	// we have received the next in order packet
	assert(packet.HasPayload());
	if(packet.HasPayload())
	{
		mUserDatagramsReceived++;
		memcpy(pBuffer, packet.Payload(), packet.PayloadSize());
		return packet.PayloadSize();
	}
	return 0;
}// END Recv



s32 cReliableUdpSocket::Send(const void* pBuffer, u32 bufSize)
{
	if(bufSize == 0 || !IsOpen())
	{
		return 0;
	}

	// we can only receive data while closing a connection, data pending on the output queue and any retransmits will still be sent but no new data
	if(IsConnectionClosing())
	{
		return 0;
	}

	// if have been sending packets faster then our peer can receive for some time and we have filled our output queue then the send fails
	if(mOutputQueue.FreeSpace() < bufSize)
	{
		return OUTPUT_QUEUE_FULL;
	}

	cReliableUdpPacket packet(0, static_cast<const u8*>(pBuffer), bufSize);

	packet.Header().sequenceNumber = IncrementSequenceNumber();

	// Data, NUL, FIN & RST segments must always piggyback an ACK
	if(mSocketState == ESTABLISHED && (packet.HasPayload() || packet.IsFlagSet(NUL) || packet.IsFlagSet(RST) || packet.IsFlagSet(FIN)))
	{
		packet.SetFlag(ACK);
		packet.Header().ackNumber = LastInOrderPacketNumberReceived();		
	}

	s32 bytesSent;
	RudpSendResult res = Send(packet, &bytesSent);

	switch(res)
	{
	case DATA_SENT:
		{	
			mUserDatagramsSent++;
			return bytesSent - sizeof(cReliableUdpPacket::RudpPacketHeader);
		}

	case DATA_QUEUED:
		{
			// the data hasn't gone yet so do not add to the unAck'd buffer, it will get added to that when it really goes
			mUserDatagramsSent++;
			return bytesSent - sizeof(cReliableUdpPacket::RudpPacketHeader);
		}

	case SEND_FAILED_WINSOCK_ERROR:
		{
			return bytesSent;
		}

	default:
		{
			assert(0);
			return SEND_FAILED_WINSOCK_ERROR;
		}
	}
}// END Send



// if flow control had kicked in and we have queued up packets to send, they get priority above everything else. we send data in the order it was passed to us.
void cReliableUdpSocket::ServiceOutputQueue()
{
	if(mOutputQueue.NumElements() ==0)
	{
		return;
	}

	cReliableUdpPacket p;

	//printf("BEGIN ServiceOutputQueue : Size %d Can send %d\n", mOutputQueue.NumElements(), NumSegmentsCanSendBeforeFlowControlStarts());

	// if flow control is not stopping us sending anything and we have segments to send ...
	while(NumSegmentsCanSendBeforeFlowControlStarts() > 0 &&
		  NextPendingPacketOnOutputQueue(&p))
	{
		if(NeedToPing())
		{
			AddPingToSegment(p);
		}

		// this segment may have been sat in the output queue for some time and its ACK may be out of date...
		if(p.IsFlagSet(ACK))
		{
			p.Header().ackNumber = LastInOrderPacketNumberReceived();
		}

		// the packet is popped off the output queue now and is ready to be sent, pump it through
		SendRudpPacket(p);

		assert(!p.IsPureAckSegment());
		if(!p.IsPureAckSegment())
		{
			bool bAdded = mUnAckdSegmentBuffer.AddSegment(mTimer.ElpasedMs(), p);
			// this packet is lost and we are in trouble if this fails
			assert(bAdded);
			assert(mUnAckdSegmentBuffer.NumElements() <= MAX_OUTSTANDING_SEGMENTS);
		}
	}

	//printf("END ServiceOutputQueue : Size %d\n", mOutputQueue.NumElements());
}// END ServiceOutputQueue



// receive up to MAX_DATAGRAMS_ON_INPUT_QUEUE pending segments from udp, send any ack's we need to
u32 cReliableUdpSocket::ServiceInputQueue()
{
	assert(ConnectionEstablished() || IsConnectionClosing());
	if(mInputQueue.NumElements() >= MAX_DATAGRAMS_ON_INPUT_QUEUE)
	{
		return 0;
	}

	cReliableUdpPacket packet;
	u32 datagramsRecvd=0;

	bool needToSendStandAloneAckSegment=false;

	if(mOutOfOrderSegmentBuffer.NumElements() == 0)
	{
		mOutOfSequenceEakCounter = 0;
	}

	// this stops us spending too much time in the following loop, if the otherside started bombarding us with dupe packets, we could potentially get stuck in the loop below forever
	u32 segmentsProcessed=0;

	// we will not receive more than MAX_DATAGRAMS_ON_INPUT_QUEUE segments, this stops the sender flooding us with segments because they won't receive an ack for anything other than
	// segments we have on our input queue
	while(mInputQueue.NumElements() < MAX_DATAGRAMS_ON_INPUT_QUEUE &&
		  segmentsProcessed < MAX_DATAGRAMS_ON_INPUT_QUEUE &&
		  RecvRudpPacket(&packet))
	{
		// valid packet received

		segmentsProcessed++;

		// no sequence number
		if(packet.IsPureAckSegment())
		{
			OnACKReceived(packet);
			continue;
		}

		// no sequence number
		if(packet.IsFlagSet(EAK))
		{
			assert(!packet.HasPayload());
			OnEAKReceived(packet);
			continue;
		}


		// catch all for bullshit packets
		if(!AreSequenceNumbersInValidRange(mNextExpectedSequenceNumber, packet.SequenceNumber()))
		{
			assert(0);
			SendRstSegment();
			CloseSocket();
		}


		// we are being ping'd, this packet has a ping piggybacked on it
		// this packet could have been sat in the udp input buffer for some time if flow control is active, this means we do not get the on the wire time but the time to get to the input queue time
		if(packet.IsFlagSet(PNG))
		{
			// can't send an ack segment from within this loop otherwise more packets can arrive because of it and we spend all our time in here
			needToSendStandAloneAckSegment = true;	
		}

		// not counting pure ack segments
		// but am counting dupe packets????
		datagramsRecvd++;


		// packet ordering is not a concern until we enter the ESTABLISHED state


		// check if this is the next packet we are expecting
		u16 seq = packet.Header().sequenceNumber;
		if(seq == mNextExpectedSequenceNumber)
		{
			OnInOrderPacketReceived(packet);
		}
		else
		{
			// out of order packet

			mOutOfOrderPacketsReceived++;

			
			u16 seqRcvd = packet.SequenceNumber();


			// TODO : if we receive an out of order packet as the seq numbers wrap the u16 we will throw away an out of order packet as a dupe. Not the end of the world.


			// is it a packet we have already ack'd or that we are holding in the out of order queue?
			if(IsDupePacket(packet))
			{
				printf("DUPE PACKET received seq %d, throwing away\n", seqRcvd);
				packet.DebugPrint();
				
				// our ack has been lost, re-ack and throw this packet away
				// can't send an ack segment from within this loop otherwise more packets can arrive because of it and we spend all our time in here
				//needToSendStandAloneAckSegment = true;	

				continue;
			}
			else
			{
				// not a dupe and not the one we want, it has to be out of order

				printf("OUT OF ORDER PACKET received seq %d. Next Expected seq = %d\n", seqRcvd, mNextExpectedSequenceNumber);
				mOutOfSequenceEakCounter++;
				assert(mOutOfOrderSegmentBuffer.RetrieveSegment(seqRcvd) == NULL);
				assert(packet.SequenceNumber() != SEQ_IGNORE);
				mOutOfOrderSegmentBuffer.AddSegment(mTimer.ElpasedMs(), packet);
				continue;
			}
		}
	}

	if(mOutOfSequenceEakCounter >= DEFAULT_OUT_OF_SEQUENCE_ACK_COUNTER)
	{
		mOutOfSequenceEakCounter=0;
		SendEakSegment();
	}
	else
	{
		// if we have more than CUMULATIVE_ACK_COUNTER number of unack'd segments, send a stand alone ack
		if( needToSendStandAloneAckSegment ||
			u16(LastInOrderPacketNumberReceived() - mLastAckNumberSent) >= CUMULATIVE_ACK_COUNTER)
		{
			SendAckSegment();
		}
	}

	return datagramsRecvd;
}// END ServiceInputQueue



// check if the next packet(s) we are expecting are pending on the out of order queue
void cReliableUdpSocket::ServiceOutOfOrderQueue()
{
	if(mOutOfOrderSegmentBuffer.NumElements() > 0)
	{		
		cSegmentBuffer::cSegmentBufferItem* pItem = mOutOfOrderSegmentBuffer.RetrieveSegment(mNextExpectedSequenceNumber);
		while(pItem)
		{
			printf("Found next expected packet (%d) on our out of order queue\n", pItem->mSegment.SequenceNumber());

			// removes the segment from the out of order buffer and if it needs to puts it into the input buffer, the sequence number is advanced
			OnInOrderPacketReceived(pItem->mSegment);		
			//mOutOfOrderSegmentBuffer.RemoveSegment(pItem->mSegment.SequenceNumber());

			pItem = mOutOfOrderSegmentBuffer.RetrieveSegment(mNextExpectedSequenceNumber);
		}
	}
}// END ServiceOutOfOrderQueue



// returns true if the packet was added to the input queue
bool cReliableUdpSocket::OnInOrderPacketReceived(const cReliableUdpPacket& packet)
{
	// in order packet, add one to the sequence number. This means that this and all packets before it have been successfully received and will be ack'd
	mNextExpectedSequenceNumber++;
	if(mNextExpectedSequenceNumber == SEQ_IGNORE)
	{
		mNextExpectedSequenceNumber++;
	}

	// process the piggy back ack now
	if(packet.IsFlagSet(ACK))
	{
		OnACKReceived(packet);
	}

	// only receive FIN, RST & NUL in order
	if(packet.IsFlagSet(FIN))
	{
		// we are using pure FIN segments only
		assert(packet.PayloadSize() == 0);
		OnFINReceived();
		return false;
	}

	if(packet.IsFlagSet(RST))
	{
		// we are using pure RST segments only
		assert(packet.PayloadSize() == 0);
		OnRSTReceived();
		return false;
	}	

	if(packet.IsFlagSet(NUL))
	{
		// we are using pure NUL segments only
		assert(packet.PayloadSize() == 0);
		OnNULReceived();
		return false;
	}


	// TODO : the meta data isn't used here. seems silly
	mInputQueue.AddSegment(0, packet);
	assert(packet.HasPayload() && !packet.IsFlagSet(NUL) && !packet.IsFlagSet(FIN) && !packet.IsFlagSet(RST));
	assert(mInputQueue.NumElements() <= MAX_DATAGRAMS_ON_INPUT_QUEUE);

	// keep the out of sequence queue up to date
	mOutOfOrderSegmentBuffer.RemoveSegment(packet.SequenceNumber());

	if(mOutOfOrderSegmentBuffer.NumElements() > 0)
	{	
		ServiceOutOfOrderQueue();
	}

	return true;
}// END OnInOrderPacketReceived



bool cReliableUdpSocket::NextPendingPacketOnInputQueue(cReliableUdpPacket* pPacketOut)
{
	if(mInputQueue.NumElements() == 0)
	{
		return false;
	}

	mInputQueue.PopFront(pPacketOut);
	return true;
}// END NextPendingPacketOnInputQueue



bool cReliableUdpSocket::NextPendingPacketOnOutputQueue(cReliableUdpPacket* pPacketOut)
{
	if(mOutputQueue.NumElements() == 0)
	{
		return false;
	}

	mOutputQueue.PopFront(pPacketOut);
	return true;
}// END NextPendingPacketOnOutputQueue



void cReliableUdpSocket::ProcessRetransmit()
{
	// TODO : TCP fast retransmit ...
	//  The fast retransmit enhancement works as follows: if a TCP sender receives three duplicate acknowledgments
	//  with the same acknowledge number (that is, a total of four acknowledgments with the same acknowledgment number),
	//  the sender can be reasonably confident that the segment with the next higher sequence number was dropped, and 
	//  will not arrive out of order. The sender will then retransmit the packet that was presumed dropped before 
	//  waiting for its timeout.

	assert(mUnAckdSegmentBuffer.NumElements() <= MAX_OUTSTANDING_SEGMENTS);
	for(u32 i=0; i < mUnAckdSegmentBuffer.NumElements(); i++)
	{
		cSegmentBuffer::cSegmentBufferItem* pSegItem = mUnAckdSegmentBuffer.RetrieveSegmentAtIndex(i);
		assert(pSegItem);

		if(pSegItem)
		{
			u32 packetAge = mTimer.ElpasedMs() - pSegItem->mPacketMetaData.mTime;

			u32 retransmissionPeriod;
			if(pSegItem->mSegment.IsFlagSet(SYN))
			{
				retransmissionPeriod = SYN_RETRANSMIT_PERIOD_MS;
			}
			else
			{
				retransmissionPeriod = max(DEFAULT_RETRANSMIT_PERIOD_MS, u32(mConnectionPing.Average() * 1.4));
			}

			if(packetAge >= retransmissionPeriod)
			{
				
				if(MAX_RETRANSMISSIONS != 0 && 
				   u32(pSegItem->mPacketMetaData.mRetransmitCount) >= MAX_RETRANSMISSIONS)
				{
					// exceeded retransmit limit, connection broken
					printf("Exceeded retransmit limit on segment %d, connection broken. It was resent %d time. Hard closing connection\n", pSegItem->mSegment.SequenceNumber(), pSegItem->mPacketMetaData.mRetransmitCount);
					CloseSocket();
					return;
				}
				else
				{
					// retransmit					
					pSegItem->mPacketMetaData.mRetransmitCount++;
					pSegItem->mPacketMetaData.mTime = mTimer.ElpasedMs();
					mPacketsRetransmitted++;

					printf("retransmitting packet %d. time = %d  last sent %dMs ago. mRetransmitCount = %d\n", pSegItem->mSegment.SequenceNumber(), mTimer.ElpasedMs(), packetAge, pSegItem->mPacketMetaData.mRetransmitCount);

					// update the ACK
					pSegItem->mSegment.Header().ackNumber = LastInOrderPacketNumberReceived();


					// jump the output queue & send now
					SendRudpPacket(pSegItem->mSegment, true);

					// maybe return from here so we don't get streams of retransmits
					//return;
				}
				
			}
		}
	}
}// END ProcessRetransmit



u16 cReliableUdpSocket::IncrementSequenceNumber()
{
	mSequenceNumber++;
	if(mSequenceNumber == SEQ_IGNORE)
	{
		mSequenceNumber++;
	}
	return mSequenceNumber;
}// END IncrementSequenceNumber



// gets the next pending packet from the datagram socket, nothing more
bool cReliableUdpSocket::RecvRudpPacket(cReliableUdpPacket* pPacketOut, cSockAddr* fromOut)
{
	s32 bytesRead = cUdpSocket::Recv(mRecvBuffer, RECV_BUFFER_SIZE, fromOut);
	if(bytesRead <= 0)
	{
		return false;
	}
	pPacketOut->SetFromRawData(mRecvBuffer, bytesRead);

	bool isPacketValid = pPacketOut->IsValidRudpPacket();


#if RUDP_PACKET_DEBUGGING
	//printf("\nPacket received t = %d:\n", mTimer.ElpasedMs());
	//pPacketOut->DebugPrint();

	HtmlPacketLogPType pType = PTYPE_RECEIVE;

	if(!isPacketValid)
	{
		pType = PTYPE_RECEIVE_CORRUPT;
	}
	else
	{
		if(IsDupePacket(*pPacketOut))
		{
			pType = PTYPE_RECEIVE_DUPE;
		}
		else if(IsOutOfOrderPacket(*pPacketOut))
		{
			if(mNextExpectedSequenceNumber == pPacketOut->SequenceNumber())
			{
				printf("a");
			}

			pType = PTYPE_RECEIVE_OUT_OF_ORDER;
		}
	}

	mPacketLog.LogPacket(mTimer.ElpasedMs(), *pPacketOut, pType);
#endif


	if(!isPacketValid)
	{
		mCorruptPacketsReceived++;

		assert(0);

		// throw away for sure but here?
		return false;
	}

	mPacketsReceived++;



	return true;
}// END RecvRudpPacket



// pumps the packet through the socket, nothing more
s32 cReliableUdpSocket::SendRudpPacket(cReliableUdpPacket& p, bool isRetransmission)
{
	if(ConnectionEstablished() && !isRetransmission)
	{
		// don't let packets jump the queue
		assert(p.SequenceNumber() == SEQ_IGNORE || p.SequenceNumber() == 0 || p.SequenceNumber() == mLastSequenceNumberSentThroughSocket+1);
	}

	if(p.SequenceNumber() != SEQ_IGNORE && !isRetransmission)
	{
		mLastSequenceNumberSentThroughSocket = p.SequenceNumber();
	}

	if(p.IsFlagSet(ACK))
	{
		mLastAckNumberSent = p.Header().ackNumber;
		mLastAckSendTime = mTimer.ElpasedMs();
	}

	if(p.HasPayload())
	{
		mLastSegmentSendTime = mTimer.ElpasedMs();

		// set the checksum
		p.Header().checksum = p.CalcChecksum();
	}

	assert(p.IsValidRudpPacket());

#if RUDP_PACKET_DEBUGGING
	//printf("\nPacket sent t = %d:\n", mTimer.ElpasedMs());
	//p.DebugPrint();
	mPacketLog.LogPacket(mTimer.ElpasedMs(), p, isRetransmission ? PTYPE_RETRANSMIT : PTYPE_SEND);
#endif



	// even when simulating network conditions the packet is now considered 'on the wire'
	mPacketsSent++;	

#if SIMULATE_NETWORK_CONDITIONS

	gNetConditionsSimulator.AddSegment(p);
	return p.PacketSize();

#else	

	// just pump our packet through the normal udp socket
	s32 bytesSent = cUdpSocket::Send(const_cast<cReliableUdpPacket&>(p).Packet(), p.PacketSize());
	assert(bytesSent == p.PacketSize());

	return bytesSent;
#endif	
}// END SendRudpPacket



// last stop before the packet is sent, sets the ack flag and potentially queues up the data if flow control is active
cReliableUdpSocket::RudpSendResult cReliableUdpSocket::Send(cReliableUdpPacket& p, s32* pBytesSentOut)
{
	// flow control, only so many segments can be sent without getting an ack back, this stops us overwhelming the other side with packets but fills up the output queue
	// if flow control is active or has been active we cannot send data until the backlog is cleared as we have to send data in the order it is passed to us
	if(IsFlowControlActive() || mOutputQueue.NumElements() > 0)
	{
		mOutputQueue.AddSegment(0, p);
		if(pBytesSentOut)
		{
			*pBytesSentOut = p.PacketSize();
		}
		printf("Flow control active, queuing SEND packet. Output queue size %d (and in bytes) %d. mSequenceNumber = %d\n", mOutputQueue.NumElements(), mOutputQueue.SizeInBytes(), mSequenceNumber);
		return DATA_QUEUED;
	}


	if(NeedToPing())
	{
		AddPingToSegment(p);
	}

	// pump our packet through the normal udp socket
	s32 bytesSent = SendRudpPacket(p);

	if(pBytesSentOut)
	{
		*pBytesSentOut = bytesSent;
	}

	if(bytesSent < 0)
	{
		return SEND_FAILED_WINSOCK_ERROR;
	}


	bool bAdded = mUnAckdSegmentBuffer.AddSegment(mTimer.ElpasedMs(), p);
	// this packet is lost and we are in trouble if this fails
	assert(bAdded && !p.IsPureAckSegment() && mUnAckdSegmentBuffer.NumElements() <= MAX_OUTSTANDING_SEGMENTS);

	return DATA_SENT;
}// END Send



u16 cReliableUdpSocket::LastInOrderPacketNumberReceived() const
{
	if(mNextExpectedSequenceNumber == SEQ_IGNORE)
	{
		return SEQ_IGNORE;
	}

	u16 seq = mNextExpectedSequenceNumber - 1;
	if(seq == SEQ_IGNORE)
	{
		seq--;
	}
	return seq;
}// END LastInOrderPacketNumberReceived



// a packet is considered a dupe if it is within the acceptable window of accepted packets and has a previous sequence number, there are issues with wrapping to consider
bool cReliableUdpSocket::IsDupePacket(const cReliableUdpPacket& p) const
{
	if(p.SequenceNumber() == SEQ_IGNORE || mNextExpectedSequenceNumber == SEQ_IGNORE)
	{
		return false;
	}

	// prevent wrap problems, the seq has just wrapped and we receive an old packet, 65534 for example. As long as its in range we consider it a dupe, if its out of range the connection will be broken
	if( p.SequenceNumber() > mNextExpectedSequenceNumber &&
		p.SequenceNumber() >= (u16(0xFFFE) - AcceptableSequenceNumberWindow()) &&
		mNextExpectedSequenceNumber <= AcceptableSequenceNumberWindow())
	{
		return true;
	}

	// either it has an old sequence number or its already on the EAK queue
	return (p.SequenceNumber() < mNextExpectedSequenceNumber ||	(mOutOfOrderSegmentBuffer.ContainsSegment(p.SequenceNumber())));
}// END IsDupePacket



bool cReliableUdpSocket::IsOutOfOrderPacket(const cReliableUdpPacket& p) const
{
	if(p.SequenceNumber() == SEQ_IGNORE || mNextExpectedSequenceNumber == SEQ_IGNORE)
	{
		return false;
	}

	// note that we class it as a dupe if its already pending on the out of order queue
	return (p.SequenceNumber() != mNextExpectedSequenceNumber && !IsDupePacket(p));
}// END IsOutOfOrderPacket



// this difference check includes both previous and future segments
bool cReliableUdpSocket::AreSequenceNumbersInValidRange(u16 seq1, u16 seq2) const
{
	u16 sequenceDifference = AbsDifferenceBetweenSequenceNumbers(seq1, seq2);
	
	if(sequenceDifference > AcceptableSequenceNumberWindow())
	{
		return false;
	}
	return true;
}// END AreSequenceNumbersInValidRange



// what do we consider an acceptable sequence number, based on pending traffic and packets on the wire
u16 cReliableUdpSocket::AcceptableSequenceNumberWindow() const
{
	// we can have mUnAckdSegmentBuffer.NumElements outstanding segments and also several segments queued up to send (each with a seq number), add MAX_OUTSTANDING_SEGMENTS*2 as a safety net for packets left floating around the internet
	return ((MAX_OUTSTANDING_SEGMENTS*2) + mUnAckdSegmentBuffer.NumElements() + mOutputQueue.NumElements());
}// END AcceptableSequenceNumberWindow



// used to help with wrapping
u16 cReliableUdpSocket::AbsDifferenceBetweenSequenceNumbers(u16 seq1, u16 seq2) const
{
	u16 sequenceDifference = abs(s16(s32(seq1) - s32(seq2)));
	return sequenceDifference;
}// END AbsDifferenceBetweenSequenceNumbers



//////////////////////////////////////////////////////////////////////////
// Control flag handlers


bool cReliableUdpSocket::NeedToPing() const
{
	if( ConnectionEstablished() &&
		!PingSegmentOutstanding() && 
		mTimer.ElpasedMs() - mLastPingSendTime >= PING_FREQUENCY_MS)
	{
		return true;
	}
	return false;
}// END NeedToPing



bool cReliableUdpSocket::AddPingToSegment(cReliableUdpPacket& segment)
{
	if(	/*IsFlowControlActive() ||*/
		(segment.HasPayload() == false && !segment.IsFlagSet(NUL)))
	{
		return false;
	}

	assert(!PingSegmentOutstanding());
	if(!PingSegmentOutstanding())
	{
		segment.Header().png = 1;
		mPingOutstanding = true;
		mLastPingSendTime = mTimer.ElpasedMs();
		mPingSegmentSeqNumber = segment.SequenceNumber();
		return true;
	}

	return false;
}// END AddPingToSegment



void cReliableUdpSocket::OnPngSegmentAck()
{
	u32 latestPing = mTimer.ElpasedMs() - mLastPingSendTime;
	mConnectionPing.Submit(latestPing);
	printf("New PING recorded. This RTT %d smoothed Ping %d\n", latestPing, mConnectionPing.Average());
	mPingOutstanding = false;
	mPingSegmentSeqNumber = SEQ_IGNORE;

	if(latestPing > mHighestLatency)
	{
		mHighestLatency = latestPing;
	}

	if(latestPing < mLowestLatency)
	{
		mLowestLatency = latestPing;
	}
}// END OnPngSegmentAck



// sends an ack segment that is not piggybacked on data
// you cannot ack an ack segment nor can an ack segment take a sequence number
void cReliableUdpSocket::SendAckSegment()
{
	cReliableUdpPacket packet(ACK, NULL, 0);	
	packet.Header().ackNumber = LastInOrderPacketNumberReceived();
	assert(packet.Header().ackNumber != SEQ_IGNORE);
	packet.Header().sequenceNumber = SEQ_IGNORE;
	
	//printf("Sending pure ack %d mInputQueue size %d", packet.Header().ackNumber, mInputQueue.NumElements());

	mLastAckNumberSent = packet.Header().ackNumber;
	mLastAckSendTime = mTimer.ElpasedMs();

	// this pumps the segment through without incrementing the sequence number or adding the segment to any buffers, jumps the queue essentially
	SendRudpPacket(packet);
}// END SendAckSegment



void cReliableUdpSocket::SendEakSegment()
{
	assert(mOutOfOrderSegmentBuffer.NumElements() > 0 && mOutOfOrderSegmentBuffer.NumElements() <= MAX_OUTSTANDING_SEGMENTS);

	if(mOutOfOrderSegmentBuffer.NumElements() > 0)
	{
		u16 eakSegmentNumbers[MAX_OUTSTANDING_SEGMENTS];
		u32 numEaks = mOutOfOrderSegmentBuffer.NumElements();

		for(u32 i=0; i < mOutOfOrderSegmentBuffer.NumElements(); i++)
		{
			cSegmentBuffer::cSegmentBufferItem* pItem = mOutOfOrderSegmentBuffer.RetrieveSegmentAtIndex(i);
			assert(pItem);
			if(pItem)
			{
				eakSegmentNumbers[i] = pItem->mSegment.SequenceNumber();
				pItem->mPacketMetaData.mEakCount++;
			}
		}

		cReliableUdpPacket packet(ACK | EAK, reinterpret_cast<u8*>(eakSegmentNumbers), numEaks*sizeof(u16));	
		packet.Header().headerLength = static_cast<u8>(sizeof(cReliableUdpPacket::RudpPacketHeader) + (numEaks*sizeof(u16)));
		packet.Header().sequenceNumber = SEQ_IGNORE;
		packet.Header().ackNumber = LastInOrderPacketNumberReceived();

		printf("Sending EAK for segments ");
		for(u32 i=0; i < numEaks; i++)
		{
			printf("%d ", eakSegmentNumbers[i]);
		}
		printf("\n");

		// this pumps the segment through without incrementing the sequence number or adding the segment to any buffers, jumps the queue essentially
		SendRudpPacket(packet);
	}
}// END SendEakSegment



void cReliableUdpSocket::SendFinSegment()
{
	printf("sending FIN\n");
	// you could piggyback the fin but i am keeping things simple, its a standalone segment
	cReliableUdpPacket packet(FIN | ACK, NULL, 0);
	packet.Header().sequenceNumber = IncrementSequenceNumber();
	packet.Header().ackNumber = LastInOrderPacketNumberReceived();
	Send(packet);
}// END SendFinSegment



void cReliableUdpSocket::SendRstSegment()
{
	// as with FIN this is a standalone segment
	cReliableUdpPacket packet(RST | ACK, NULL, 0);
	packet.Header().sequenceNumber = IncrementSequenceNumber();
	packet.Header().ackNumber = LastInOrderPacketNumberReceived();
	Send(packet);
}// END SendRstSegment



bool cReliableUdpSocket::SendNulSegment()
{
	if(	IsFlowControlActive())
	{
		return false;
	}

	cReliableUdpPacket packet(NUL | ACK, NULL, 0);
	packet.Header().sequenceNumber = IncrementSequenceNumber();
	packet.Header().ackNumber = LastInOrderPacketNumberReceived();
	mLastSegmentSendTime = mTimer.ElpasedMs();
	Send(packet);
	return true;
}// END SendNulSegment



void cReliableUdpSocket::OnACKReceived(const cReliableUdpPacket& segment)
{
	u16 sequenceNumberAcked = segment.Header().ackNumber;

	assert(mUnAckdSegmentBuffer.NumElements() <= MAX_OUTSTANDING_SEGMENTS);

	// check that the sequence number they are acking is valid
	// we have mUnAckdSegmentBuffer.NumElements outstanding segments and also several segments queued up to send (each with a seq number), add MAX_OUTSTANDING_SEGMENTS as a safety net for packets left floating around the internet
	if(	sequenceNumberAcked == SEQ_IGNORE ||
		!AreSequenceNumbersInValidRange(mSequenceNumber, sequenceNumberAcked))
	{
		printf("BAD ACK. recvd %d ours %d diff %d. Outstanding segments %d Queued to send %d\n", sequenceNumberAcked, mSequenceNumber, AbsDifferenceBetweenSequenceNumbers(mSequenceNumber, sequenceNumberAcked), mUnAckdSegmentBuffer.NumElements(), mOutputQueue.NumElements());

		// this can be:
		// (i)   transport bug - connection is boned
		// (ii)  packet has managed to float around the internet for longer than it took us to retransmit the packet, get the ack back, send anything on output queue and send 
		//       another MAX_OUTSTANDING_SEGMENTS. Very very rare, if it happens lets just close the connection
		// (iii) Data left over from an old connection (1 in 65535 chance as we rand the seq num) or someone injecting packets to hack us, either way close the connection

		// TODO : close
		assert(0);

		CloseSocket();
		return;
	}

	u32 segmentsRemoved = mUnAckdSegmentBuffer.RemoveAllSegmentsSentBefore(sequenceNumberAcked);

	// remove any segments that are queued for retransmission
	segmentsRemoved = mOutputQueue.RemoveAllSegmentsSentBefore(sequenceNumberAcked);
	assert(segmentsRemoved == 0);


	// check if our ping has returned
	if(PingSegmentOutstanding() && mUnAckdSegmentBuffer.RetrieveSegment(mPingSegmentSeqNumber)==NULL)
	{
		OnPngSegmentAck();
	}


#if RUDP_PACKET_DEBUGGING
	//printf("ACK received for segment %d\n", sequenceNumberAcked);
	//printf("removed %d segments. unACKd segment count %d\n", segmentsRemoved, mUnAckdSegmentBuffer.NumElements());
	//printf("current sequence number = %d Outstanding segments %d Queued to send %d\n", mSequenceNumber, mUnAckdSegmentBuffer.NumElements(), mOutputQueue.NumElements());
#endif
}// END OnACKReceived



void cReliableUdpSocket::OnEAKReceived(const cReliableUdpPacket& segment)
{
	assert(segment.IsFlagSet(ACK));
	if(segment.IsFlagSet(ACK))
	{
		OnACKReceived(segment);
	}

	// figure out which segment(s) got lost and resend now
	u32 numSegmentsEakd = (segment.Header().headerLength - sizeof(cReliableUdpPacket::RudpPacketHeader)) / sizeof(u16);
	const u16* eakSegmentNumbers = reinterpret_cast<const u16*> (segment.Packet() + sizeof(cReliableUdpPacket::RudpPacketHeader));

	assert(mUnAckdSegmentBuffer.NumElements() <= MAX_OUTSTANDING_SEGMENTS);


	// TODO : wrap problems here !
	u32 highestEakRecvd=0;

	printf("OnEAKReceived for segments ");
	for(u32 i=0; i < numSegmentsEakd; i++)
	{
		if(eakSegmentNumbers[i] > highestEakRecvd)
		{
			highestEakRecvd = eakSegmentNumbers[i];
		}

		printf("%d ", eakSegmentNumbers[i]);
	}
	printf("\n");



	// go through our unack buffer and retransmit the lost segments and reset the retransmit timer on the eak'd segments
	for(u32 i=0; i < mUnAckdSegmentBuffer.NumElements(); i++)
	{
		cSegmentBuffer::cSegmentBufferItem* pItem = mUnAckdSegmentBuffer.RetrieveSegmentAtIndex(i);

		assert(pItem);
		if(pItem)
		{
			// we only retransmit a certain number of times when we get an EAK. The reason for this is that generally when we  start getting EAK's the list starts small but quickly
			// grows as more packets arrive. Therefore the first out of order packet starts getting EAK'd again and again and retransmits start going crazy.
			if(pItem->mPacketMetaData.mEakCount >= DEFAULT_MAX_EAK)
			{
				continue;
			}

			bool segEakd=false;
			for(u32 j=0; j < numSegmentsEakd; j++)
			{
				if(eakSegmentNumbers[j] == pItem->mSegment.SequenceNumber())
				{
					segEakd=true;
				}
			}

			if(segEakd)
			{
				printf("Packet %d EAK'd, resetting retransmit timer\n", pItem->mSegment.SequenceNumber());
				pItem->mPacketMetaData.mTime = mTimer.ElpasedMs();
			}
			else
			{
				// don't retransmit segments that was sent after the eak we just received
				if(pItem->mSegment.SequenceNumber() < highestEakRecvd)
				{
					//missing packet, retransmit
					pItem->mPacketMetaData.mRetransmitCount++;
					pItem->mPacketMetaData.mTime = mTimer.ElpasedMs();
					mPacketsRetransmitted++;

					printf("EAK detects lost packet. Retransmitting packet %d. time = %d. mRetransmitCount = %d\n", pItem->mSegment.SequenceNumber(), mTimer.ElpasedMs(), pItem->mPacketMetaData.mRetransmitCount);

					// update the ACK
					pItem->mSegment.Header().ackNumber = LastInOrderPacketNumberReceived();

					// jump the output queue & send now
					SendRudpPacket(pItem->mSegment, true);

					// count how many times we have resent this segment due to an EAK
					pItem->mPacketMetaData.mEakCount++;
				}
			}
		}
	}
}// END OnEAKReceived



void cReliableUdpSocket::OnSYNReceived(const cReliableUdpPacket& synSegment)
{
	// sync the sequence number
	mNextExpectedSequenceNumber = synSegment.SequenceNumber() + 1;
	if(mNextExpectedSequenceNumber == SEQ_IGNORE)
	{
		mNextExpectedSequenceNumber++;
	}
}// END OnSYNReceived



void cReliableUdpSocket::OnFINReceived()
{
	printf("FIN Received\n");

	// we ACK a FIN no matter what
	SendAckSegment();

	switch(mSocketState)
	{
	// nothing to do
	case CLOSED:
	case LISTEN:
		break;

	// FIN here means someone isn't happy with us, close com's now
	case SYN_SENT:
	case SYN_RECEIVED:	
	case TIME_WAIT:
		CloseSocket();
		break;

	// error : double FIN, should never happen
	case CLOSE_WAIT:
	case LAST_ACK:
		assert(0);
		CloseSocket();
		break;
	

	// when closing a connection in a reasonable manner, we will be on one of the 3 states below ...

	// otherside has sent a FIN, send what we have left then send our fin
	case ESTABLISHED:
		State(CLOSE_WAIT);
		break;


	// active close
	case FIN_WAIT_1:
		{
			// Both sides have sent a FIN so we only have one state to go to -> TIME_WAIT to wait for any remaining ACKs to arrive.
			// note that because of the way we queue incoming packets the ACK for our FIN and their FIN may have been queued up on our input queue together
			// This results in us getting into this active close situation quite frequently on LAN's
			State(TIME_WAIT);
			mTimeEnteredTimeWaitState = mTimer.ElpasedMs();
		}
		break;


	// passive close
	case FIN_WAIT_2:
		State(TIME_WAIT);
		mTimeEnteredTimeWaitState = mTimer.ElpasedMs();
		break;

	default:
		assert(0);
	}
}// END OnFINReceived



void cReliableUdpSocket::OnRSTReceived()
{
	printf("RST Received\n");

	//SendAckSegment();

	switch(mSocketState)
	{
	// nothing to do
	case CLOSED:
	case LISTEN:
		break;

	// all the following states are after we have called connect() on the udp socket and therefore it means it is the otherside that has sent the rst
	case SYN_SENT:
	case SYN_RECEIVED:	
	case TIME_WAIT:
	case CLOSE_WAIT:
	case LAST_ACK:
	case ESTABLISHED:
	case FIN_WAIT_1:
	case FIN_WAIT_2:
		CloseSocket();
		break;

	default:
		assert(0);
	}
}// END OnRSTReceived



void cReliableUdpSocket::OnNULReceived()
{
	// immediately ACK a keep alive segment
	SendAckSegment();
}// END OnNULReceived





//////////////////////////////////////////////////////////////////////////
// States 


std::string cReliableUdpSocket::StateAsString(SocketState state) const
{
	switch(state)
	{
	case CLOSED:
		return "CLOSED";

	case LISTEN:
		return "LISTEN";

	case SYN_SENT:
		return "SYN_SENT";

	case SYN_RECEIVED:
		return "SYN_RECEIVED";

	case ESTABLISHED:
		return "ESTABLISHED";

	case CLOSE_WAIT:
		return "CLOSE_WAIT";

	case LAST_ACK:
		return "LAST_ACK";

	case FIN_WAIT_1:
		return "FIN_WAIT_1";

	case FIN_WAIT_2:
		return "FIN_WAIT_2";

	case TIME_WAIT:
		return "TIME_WAIT";

	default:
		assert(0);
	}

	return "ERROR";
}// END StateAsString



void cReliableUdpSocket::State(SocketState state)
{
	if(state != mSocketState)
	{
		printf("Socket State %s -> %s\n", StateAsString(mSocketState).c_str(), StateAsString(state).c_str());
		mSocketState = state;
	}
}// END State



void cReliableUdpSocket::ProcessState_SynSent()
{
	cReliableUdpPacket packet;
	if(RecvRudpPacket(&packet))
	{
		if(packet.IsFlagSet(RST))
		{
			OnRSTReceived();
			return;
		}

		if(packet.IsFlagSet(SYN))
		{
			OnSYNReceived(packet);

			// send ack 
			printf("Acking SYN from %s\n", mConnectedAddress.Ip().AsString());
			SendAckSegment();

			// 3 way handshake, make sure our SYN was ack'd either here or in the lost SYN situation below
			if(packet.IsFlagSet(ACK) || mUnAckdSegmentBuffer.NumElements() == 0)
			{	
				OnACKReceived(packet);
				State(ESTABLISHED);
			}
			// active open, we both sent SYN at the the same time or our SYN has been lost 
			else
			{
				State(SYN_RECEIVED);
			}
		}
		else
		{
			// the otherside has had a SYN was lost and is therefore only ack'ing our SYN, the retransmit will send the syn along soon
			if(mUnAckdSegmentBuffer.NumElements() == 1 && packet.IsFlagSet(ACK) && mUnAckdSegmentBuffer.RetrieveSegmentAtIndex(0)->mSegment.IsFlagSet(SYN))
			{
				OnACKReceived(packet);
			}
			else
			{
				// send RST & close socket
				SendRstSegment();
				CloseSocket();

				// debug
				assert(0);
			}
		}
	}
}// END ProcessState_SynSent



void cReliableUdpSocket::ProcessState_SynReceived()
{	
	cReliableUdpPacket packet;
	if(RecvRudpPacket(&packet))
	{
		if(packet.IsFlagSet(RST))
		{
			OnRSTReceived();
			return;
		}

		if(packet.SequenceNumber() != mNextExpectedSequenceNumber)
		{
			return;
		}

		if(packet.IsFlagSet(ACK))
		{
			OnACKReceived(packet);
		}
	}

	// any outstanding packets waiting to be ack'd? we have only sent one packet at this point so when theres nothing to ack we are good
	if(mUnAckdSegmentBuffer.NumElements()==0)
	{
		State(ESTABLISHED);
	}
}// END ProcessState_SynReceived



void cReliableUdpSocket::ProcessState_Established()
{
	if(!IsFlowControlActive() && (mTimer.ElpasedMs() - mLastSegmentSendTime >= DEFAULT_NUL_SEGMENT_TIMER_MS))
	{
		SendNulSegment();
	}
}// END ProcessState_Established



void cReliableUdpSocket::ProcessState_CloseWait()
{
	// send what we have left, wait for it to be ack'd then send our fin
	if(mOutputQueue.NumElements() == 0 && mUnAckdSegmentBuffer.NumElements() == 0)
	{
		// all data sent & ack'd now, send our FIN
		SendFinSegment();
		State(LAST_ACK);
	}
}// END ProcessState_CloseWait



void cReliableUdpSocket::ProcessState_LastAck()
{
	assert(mOutputQueue.NumElements() == 0);

	// like the state say's, we are just waiting for the ack to our RST then we can close
	if(mUnAckdSegmentBuffer.NumElements() == 0)
	{
		// all done
		CloseSocket();
	}
}// END ProcessState_LastAck



// we sit in this state until our FIN has been ack'd (passive close) or we receive a FIN from the otherside (active close)
void cReliableUdpSocket::ProcessState_FinWait1()
{
	// wait for our FIN to be ack'd, since it was the last thing we sent we need to have everything ack'd before it is
	if(mOutputQueue.NumElements() == 0 && mUnAckdSegmentBuffer.NumElements() == 0)
	{
		// we will sit in FIN_WAIT_2 until we get a FIN from the otherside, when we do we ack it and enter the time wait state, all this is done in OnFinReceive
		State(FIN_WAIT_2);
	}
}// END ProcessState_FinWait1



void cReliableUdpSocket::ProcessState_TimeWait()
{
	// wait for all segments to drain out of the network, ack's can still be sent if we receive packets
	if(mTimer.ElpasedMs() - mTimeEnteredTimeWaitState >= DEFAULT_TIMEWAIT_LENGTH_MS)
	{
		CloseSocket();			
	}	
}// END ProcessState_TimeWait



} //namespace net

