#ifndef _DHT_PACKET_LOG_H
#define _DHT_PACKET_LOG_H


#include "Network/HtmlPacketLog.h"

#include "Network/BitTorrent/dht/KrpcMsg.h"


#define DHT_PACKET_DUMP_FOLDER "\\Logs\\DhtLogs"



class cDhtPacketLog  : public cHtmlPacketLog
{
public:

	cDhtPacketLog();
	~cDhtPacketLog();

	/*
	typedef enum
	{
		DHT_PING =0,
		DHT_FINDNODE,
		DHT_GETPEERS,
		DHT_ANNOUNCEPEER,

		DHT_RESPONSE,

		DHT_ERROR
	}MessageType;
	*/

	void LogKrpcMessage(u32 time, const net::cSockAddr&, const cKrpcMsg& message/*, MessageType messageType*/, HtmlPacketLogPType packetType);

private:

	std::string FormatCellForPacket(u32 time, const net::cSockAddr& addr, const cKrpcMsg& message, /*MessageType messageType, */HtmlPacketLogPType packetType);
};





#endif // _DHT_PACKET_LOG_H