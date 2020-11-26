// Jon Bellamy



#if USE_PCH
#include "stdafx.h"
#endif


#include "ReliableUdpPacket.h"


#include <memory.h>

#include "Network/NetSettings.h"



namespace net {



// leave the packet uninitialised 
cReliableUdpPacket::cReliableUdpPacket()
: mpDataSize(0)
, mpData(NULL)
{
}// END cReliableUdpPacket



// create a new packet with the passed data and flags
cReliableUdpPacket::cReliableUdpPacket(u8 flags, const u8* payload, u32 payloadSize)
: mpDataSize(0)
, mpData(NULL)
{
	AllocData(payload, payloadSize);

	Header().packetFlags = flags;
	
	// TODO 
	Header().sequenceNumber = 0;
	Header().ackNumber = 0;
	Header().checksum = 0x1234;
}// END cReliableUdpPacket



// create a packet with the passed capacity (room for the header will also be allocated). Everything will be 0
cReliableUdpPacket::cReliableUdpPacket(u32 payloadSize)
: mpDataSize(0)
, mpData(NULL)
{
	AllocData(NULL, payloadSize);
	
	Header().checksum = 0x1234;
}// END cReliableUdpPacket




cReliableUdpPacket::cReliableUdpPacket(const cReliableUdpPacket& rhs)
: mpDataSize(0)
, mpData(NULL)
{
	*this = rhs;
}// END cReliableUdpPacket


cReliableUdpPacket::~cReliableUdpPacket()
{
	FreeData();
}// END ~cReliableUdpPacket



cReliableUdpPacket& cReliableUdpPacket::operator= (const cReliableUdpPacket& rhs)
{
	FreeData();
	mpDataSize = rhs.mpDataSize;
	mpData = new u8 [mpDataSize];
	memcpy(mpData, rhs.mpData, mpDataSize);
	return *this;
}// END operator=



// this assumes that the u8* bytes is a genuine packet including header, NOT just payload data
void cReliableUdpPacket::SetFromRawData(const u8* bytes, u32 size)
{
	FreeData();
	mpDataSize = size;
	mpData = new u8[mpDataSize];
	memcpy(mpData, bytes, mpDataSize);
}// END SetFromRawData



void cReliableUdpPacket::AllocData(const u8* payload, u32 payloadSize)
{
	assert(mpData == NULL);

	mpDataSize = payloadSize + sizeof(RudpPacketHeader);

	// do we really want to new this? pool, standard size or realloc or some such required i think
	mpData = new u8[mpDataSize];

	memset(&Header(), 0, sizeof(RudpPacketHeader));
	Header().headerLength = sizeof(RudpPacketHeader);

	if(payload)
	{	
		memcpy(Payload(), payload, payloadSize);
	}
}// END AllocData



void cReliableUdpPacket::FreeData()
{
	if(mpData)
	{
		delete[] mpData;
		mpData = NULL;
		mpDataSize = 0;
	}
}// END FreeData


u16 cReliableUdpPacket::CalcChecksum() const
{
	// http://rfc.sunsite.dk/rfc/rfc1071.html
	// Compute Internet Checksum for "count" bytes beginning at location "addr".

	u32 sum = 0;
	u32 count = PayloadSize();
	const u16* addr = reinterpret_cast<const u16*> (Payload());

	while(count > 1)  
	{
		sum += *addr++;
		count -= 2;
	}

	//  Add left-over byte, if any 
	if( count > 0 )
	{
		sum += * (u8*) addr;
	}

	//  Fold 32-bit sum to 16 bits 
	while (sum>>16)
	{
		sum = (sum & 0xffff) + (sum >> 16);
	}

	u16 checksum = ~sum;
	return checksum;
}// END CalcChecksum




bool cReliableUdpPacket::IsValidRudpPacket() const
{
	if(Header().packetFlags == 0)
	{
		return false;
	}

	// checksum
	if(HasPayload())
	{
		if(Header().checksum != CalcChecksum())
		{
			assert(0);
			return false;
		}
	}
	else
	{
		if(Header().checksum != 0x1234)
		{
			assert(0);
			return false;
		}
	}




	// SYN, EACK, FIN, RST & TCS are mutually exclusive, lets check that first 
	

	u8 byteToCheck = Header().packetFlags;

	// mask out the bits we are not checking
	byteToCheck &= ~ACK;
	byteToCheck &= ~NUL;
	byteToCheck &= ~PNG;
	byteToCheck &= ~CHK;

	// Counting bits set, Brian Kernighan's way 
	u8 numberOfBitsSet; 
	for (numberOfBitsSet = 0; byteToCheck != 0; numberOfBitsSet++)
	{
		// clear the least significant bit set
		byteToCheck &= byteToCheck - 1; 
	}

	if(numberOfBitsSet > 1)
	{
		// bad packet
		return false;
	}

	// EAK must have an ACK as well
	if(IsFlagSet(EAK) && !IsFlagSet(ACK))
	{
		return false;
	}

	return true;
}// END IsValidRudpPacket



//////////////////////////////////////////////////////////////////////////
// Debug


std::string cReliableUdpPacket::PacketFlagAsString(PacketFlag f) const
{
	switch(f)
	{
	case SYN:
		return "SYN";

	case ACK:
		return "ACK";

	case EAK:
		return "EAK";

	case RST:
		return "RST";

	case FIN:
		return "FIN";

	case NUL:
		return "NUL";

	case CHK:
		return "CHK";

	case PNG:
		return "PNG";
//	case TCS:
//		return "TCS";

	default:
		assert(0);
	}

	return "ERROR";
}// END PacketFlagAsString



void cReliableUdpPacket::DebugPrint() const
{
	printf("FLAGS - ", Header().sequenceNumber);

	for(u32 i=0; i < NUM_RUDP_PACKET_FLAGS; i++)
	{
		if(IsFlagSet(PacketFlag(1<<i)))
		{
			printf("%s ", PacketFlagAsString(PacketFlag(1<<i)).c_str());
		}
	}
	printf("\n");

	printf("seq - %u\n", Header().sequenceNumber);
	printf("ack - %u\n", Header().ackNumber);
	printf("chk - 0x%X\n", Header().checksum);
	printf("pay - %u\n\n", PayloadSize());
}// END DebugPrint


//////////////////////////////////////////////////////////////////////////

} //namespace net

