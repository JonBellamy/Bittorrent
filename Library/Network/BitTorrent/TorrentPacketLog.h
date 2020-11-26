#ifndef _TORRENT_PACKET_LOG_H
#define _TORRENT_PACKET_LOG_H


#include "Network/HtmlPacketLog.h"



#define TORRENT_PACKET_DUMP_FOLDER "\\Logs\\PeerLogs"


class cTorrentPacketLog  : public cHtmlPacketLog
{
public:

	cTorrentPacketLog();
	~cTorrentPacketLog();

	typedef enum
	{
		STANDARD_MESSAGE=0,
		INCOMING_CONNECTION,
		OUTGOING_CONNECTION,
		HANDSHAKE,
		CLOSE_CONNECTION,
		BAD_MESSAGE_CLOSE_CONNECTION
	}MessageType;

	void LogStandardMessage(u32 time, const void* pMessage, MessageType messageType, HtmlPacketLogPType packetType);
	void LogNote(u32 time, const char* strNote);

private:

	std::string FormatCellForPacket(u32 time, const void* pMessage, MessageType messageType, HtmlPacketLogPType packetType);
	//void DumpPacketToDisk(u32 time, const net::iNetPacket& packet, HtmlPacketLogPType packetType);
};





#endif // _TORRENT_PACKET_LOG_H