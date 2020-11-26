// Jon Bellamy


#ifndef IPACKET_H
#define IPACKET_H


#include <string>

#include "Network/NetSettings.h"


namespace net {




class iNetPacket
{
public:

	virtual u32 PacketSize() const =0;
	virtual const u8* Packet() const =0;


//#if RUDP_PACKET_DEBUGGING
	//////////////////////////////////////////////////////////////////////////
	// debug logging functions	
	virtual std::string LogHtmlPacket(u32 time, u32 packetType) const=0;			// packetType is a HtmlPacketLogPType
	virtual void DumpPacketToDisk(u32 time, u32 packetType) =0;
	virtual std::string GetPacketFileName(u32 packetType) const =0;
	//////////////////////////////////////////////////////////////////////////
//#endif

private:


};



} // namespace net



#endif // IPACKET_H
