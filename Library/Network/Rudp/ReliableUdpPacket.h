// Jon Bellamy


#ifndef RUDP_PACKET_H
#define RUDP_PACKET_H



#include <string>
#include <assert.h>


namespace net {



typedef enum
{
	SYN = 1<<0,
	ACK = 1<<1,
	EAK = 1<<2,
	RST = 1<<3,
	FIN = 1<<4,
	NUL = 1<<5,
	CHK = 1<<6,
	PNG = 1<<7,
//	TCS = 1<<7,

	NUM_RUDP_PACKET_FLAGS = 8
}PacketFlag;



class cReliableUdpPacket
{
public:

	cReliableUdpPacket();

	// create a new packet with the passed data and flags
    cReliableUdpPacket(u8 flags, const u8* payload, u32 payloadSize);
	
	// create a packet with the passed capacity (room for the header will also be allocated). Everything will be 0
	cReliableUdpPacket(u32 payloadSize);

	cReliableUdpPacket(const cReliableUdpPacket& rhs);

	~cReliableUdpPacket();

	cReliableUdpPacket& operator= (const cReliableUdpPacket& rhs);


	typedef struct  
	{
		union
		{
			struct  
			{	
				u8 syn : 1;				// synchronization segment
				u8 ack : 1;				// acknowledgment number in header is valid
				u8 eak : 1;				// extended ACK segment is present 				
				u8 rst : 1;				// reset segment
				u8 fin : 1;				// FIN segment for gracefully closing a connection
				u8 nul : 1;				// NULL segment 
				u8 chk : 1;				// 0 = checksum is for header only. 1 = checksum is for header and data
				u8 png : 1;				// ping segment
//				u8 tcs : 1;				// transfer connection state
				// SYN, EACK, FIN, RST & TCS are mutually exclusive
			};
			u8 packetFlags;
		};

		u8 headerLength;
		u16 sequenceNumber;
		u16 ackNumber;
		u16 checksum;
	}RudpPacketHeader;


	u16 CalcChecksum() const;

	bool IsValidRudpPacket() const;

	void SetFromRawData(const u8* bytes, u32 size);

	void AllocData(const u8* payload, u32 payloadSize);
	void FreeData();



	//////////////////////////////////////////////////////////////////////////
	// inlines

	bool IsFlagSet(PacketFlag flag) const;
	void SetFlag(PacketFlag flag);

	bool IsPureAckSegment() const;

	u16 SequenceNumber() const;

	u32 PacketSize() const;
	u32 PayloadSize() const;
	bool HasPayload() const;

	const u8* Packet() const;
	
	u8* Payload();
	const u8* Payload() const;

	RudpPacketHeader& Header();
	const RudpPacketHeader& Header() const;


	//////////////////////////////////////////////////////////////////////////
	// Debug

	std::string PacketFlagAsString(PacketFlag f) const;
	void DebugPrint() const;

	//////////////////////////////////////////////////////////////////////////

private:

	union
	{
		mutable u8* mpData;					// holds the payload and the header, this is what we actually send
		RudpPacketHeader* mpHeader;
	};
	u32 mpDataSize;						// bytes allocated for mpData 
};



//////////////////////////////////////////////////////////////////////////
// inlines


inline u16 cReliableUdpPacket::SequenceNumber() const
{
	return Header().sequenceNumber;
}// END SequenceNumber



inline bool cReliableUdpPacket::IsFlagSet(PacketFlag flag) const
{
	return (Header().packetFlags & flag) != 0;
}// END IsFlagSet



inline void cReliableUdpPacket::SetFlag(PacketFlag flag)
{
	Header().packetFlags |= flag;
}// END SetFlag



inline bool cReliableUdpPacket::IsPureAckSegment() const
{
	return (PayloadSize() == 0 && ((Header().packetFlags & 0xff) == ACK));
}// END IsPureAckSegment




inline cReliableUdpPacket::RudpPacketHeader& cReliableUdpPacket::Header()
{
	assert(mpData);
	return reinterpret_cast<RudpPacketHeader&> (mpData[0]);
}// END Header



inline const cReliableUdpPacket::RudpPacketHeader& cReliableUdpPacket::Header() const
{
	assert(mpData);
	return reinterpret_cast<const RudpPacketHeader&> (mpData[0]);
}// END Header



inline u32 cReliableUdpPacket::PacketSize() const 
{ 
	return mpDataSize; 
}// END PacketSize



inline u32 cReliableUdpPacket::PayloadSize() const
{ 
	assert(mpData);
	return mpDataSize - Header().headerLength;
}// END PayloadSize



inline bool cReliableUdpPacket::HasPayload() const
{ 
	return PayloadSize() != 0; 
}// END HasPayload



inline const u8* cReliableUdpPacket::Packet() const
{
	return mpData;
}// END Packet



inline const u8* cReliableUdpPacket::Payload() const
{
	assert(mpDataSize > sizeof(RudpPacketHeader));
	return mpData + Header().headerLength;
}// END Payload



inline u8* cReliableUdpPacket::Payload()
{
	assert(mpDataSize > sizeof(RudpPacketHeader));

	// Header().headerLength needs to be considered here!
	return mpData + Header().headerLength;
}// END Payload



} // namespace net



#endif // RUDP_PACKET_H
