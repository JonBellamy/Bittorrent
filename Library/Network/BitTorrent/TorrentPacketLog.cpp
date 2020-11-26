// html table to show the flow of packets


#if USE_PCH
#include "stdafx.h"
#endif


#include "TorrentPacketLog.h"

#include <string>
#include <stdio.h>
#include <assert.h>

#include "File/FileHelpers.h"
#include "Network/BitTorrent/BitTorrentMessages.h"
#include "Network/BitTorrent/BitTorrentManager.h"


using namespace net;



cTorrentPacketLog::cTorrentPacketLog()
{
}// END cTorrentPacketLog



cTorrentPacketLog::~cTorrentPacketLog()
{
}// END ~cTorrentPacketLog



void cTorrentPacketLog::LogStandardMessage(u32 time, const void* pMessage, MessageType messageType, HtmlPacketLogPType packetType)
{
	if(IsOpen() == false)
	{
		return;
	}

	mPacketsLogged++;

	std::string html = " \r\n\
				<tr> \r\n\
					<td bgcolor=\"\" align=\"left\" width=\"200\"></td> \r\n\
					<td bgcolor=\"\" align=\"left\" width=\"200\"></td> \r\n\
					<td bgcolor=\"white\" align=\"left\" valign=\"top\" width=\"200\"></td> \r\n\
				</tr> \r\n";

	u32 pos;

	std::string cellTag("</td>");
	if(IsSendPacket(packetType))
	{
		// first col
		pos = static_cast<u32>(html.find(cellTag));
		assert(pos != std::string::npos);
	}
	else
	{
		// second col
		pos = static_cast<u32>(html.find(cellTag));
		pos = static_cast<u32>(html.find(cellTag, pos + cellTag.size()));
		assert(pos != std::string::npos);
	}	


	html.insert(pos, FormatCellForPacket(time, pMessage, messageType, packetType));


	// set the colour
	std::string col1("white");
	std::string col2("white");
	if(IsSendPacket(packetType))
	{
		col1 = GetPacketColour(packetType);
	}
	else
	{
		col2 = GetPacketColour(packetType);
	}

	// send cell
	std::string colourTag("bgcolor=\"");
	pos = static_cast<u32>(html.find(colourTag));
	assert(pos != std::string::npos);
	pos += static_cast<u32>(colourTag.size());
	html.insert(pos, col1.c_str());

	// receive cell
	pos = static_cast<u32>(html.find(colourTag, pos));
	assert(pos != std::string::npos);
	pos += static_cast<u32>(colourTag.size());
	html.insert(pos, col2.c_str());



	// connection meta data
	//if(packetType == PTYPE_RETRANSMIT ||
  	//   packetType == PTYPE_RECEIVE_DUPE ||
	//   packet.IsFlagSet(PNG))
//	if(mPacketsLogged % 10 == 0)
//	{
		// third col
//		pos = static_cast<u32>(html.rfind(cellTag));
//		assert(pos != std::string::npos);

//		char buf[128];
//		sprintf(buf, "Connection Ping %d", mSocket->mConnectionPing.Average());
//		html.insert(pos, buf);
//	}	


	bool bRet = Write(mWritePosition, static_cast<u32>(html.size()), reinterpret_cast<const u8*>(html.c_str()));
	assert(bRet);

	mWritePosition += static_cast<u32>(html.size());

	// TODO : don't want to lose data but this will thrash the disk
	//Flush();

//	DumpPacketToDisk(time, packet, packetType);
}// END LogPacket



void cTorrentPacketLog::LogNote(u32 time, const char* strNote)
{
	if(IsOpen() == false)
	{
		return;
	}

	mPacketsLogged++;

	std::string html = " \r\n\
					   <tr> \r\n\
					   <td bgcolor=\"\" align=\"left\" width=\"200\"></td> \r\n\
					   <td bgcolor=\"\" align=\"left\" width=\"200\"></td> \r\n\
					   <td bgcolor=\"white\" align=\"left\" valign=\"top\" width=\"200\"></td> \r\n\
					   </tr> \r\n";

	u32 pos;

	std::string cellTag("</td>");
	
	// first col
	pos = static_cast<u32>(html.find(cellTag));
	assert(pos != std::string::npos);


	std::string details;
	details += "<B>";	// bold
	details += "Note:";
	details += "</B>";

	details += "<BR>";	// new line
	details += strNote;

	html.insert(pos, details);


	// set the colour
	std::string col1("white");
	std::string colourTag("bgcolor=\"");
	pos = static_cast<u32>(html.find(colourTag));
	assert(pos != std::string::npos);
	pos += static_cast<u32>(colourTag.size());
	html.insert(pos, col1.c_str());


	bool bRet = Write(mWritePosition, static_cast<u32>(html.size()), reinterpret_cast<const u8*>(html.c_str()));
	assert(bRet);

	mWritePosition += static_cast<u32>(html.size());

	// TODO : don't want to lose data but this will thrash the disk
	//Flush();
}// END LogNote



// some vile void ptr stuff going on here
std::string cTorrentPacketLog::FormatCellForPacket(u32 time, const void* pMessage, MessageType messageType, HtmlPacketLogPType packetType)
{
	std::string details;
	
	if(messageType==STANDARD_MESSAGE)
	{
		const cTorrentMessage* message = reinterpret_cast<const cTorrentMessage*> (pMessage);
		
		switch(message->Type())
		{
		case TM_CHOKE:
		{
			details += "<B>";	// bold
			details += "CHOKE";
			details += "</B>";
		}
			break;
		case TM_UNCHOKE:
		{
			details += "<B>";
			details += "UNCHOKE";
			details += "</B>";
		}
			break;
		case TM_INTERESTED:
		{
			details += "<B>";
			details += "INTERESTED";
			details += "</B>";
		}
			break;
		case TM_NOT_INTERESTED:
		{
			details += "<B>";
			details += "NOT_INTERESTED";
			details += "</B>";
		}
			break;
		case TM_HAVE:
		{
			const cTorrentMessageHave* m = static_cast<const cTorrentMessageHave*> (message);
			details += "<B>";
			details += "HAVE";
			details += "</B>";	// /bold
			details += "<BR>";	// new line
			char str[128];
			sprintf(str, "mPieceIndex = %u", m->mPieceIndex);
			details += str;
		}
			break;
		case TM_BITFIELD:
		{
			details += "<B>";
			details += "BITFIELD";
			details += "</B>";	// /bold
			details += "<BR>";	// new line
		}
			break;
		case TM_REQUEST:
		{
			const cTorrentMessageRequest* m = static_cast<const cTorrentMessageRequest*> (message);
			details += "<B>";
			details += "REQUEST";
			details += "</B>";	// /bold
			details += "<BR>";	// new line
			char str[128];
			sprintf(str, "mPieceIndex = %u", m->mPieceIndex);
			details += str;
			details += "<BR>";
			sprintf(str, "mBegin = %u", m->mBegin);
			details += str;
			details += "<BR>";
			sprintf(str, "mLength = %u", m->mLength);
			details += str;
		}
			break;
		case TM_PIECE:
			{	
			const cTorrentMessagePiece* m = static_cast<const cTorrentMessagePiece*> (message);
			details += "<B>";
			details += "PIECE";
			details += "</B>";	// /bold
			details += "<BR>";	// new line

			char str[128];
			sprintf(str, "mPieceIndex = %u", m->mPieceIndex);
			details += str;
			details += "<BR>";
			sprintf(str, "mBegin = %u", m->mBegin);
			details += str;
		}
			break;
		case TM_CANCEL:
		{
			const cTorrentMessageCancel* m = static_cast<const cTorrentMessageCancel*> (message);
			details += "<B>";
			details += "CANCEL";
			details += "</B>";	// /bold
			details += "<BR>";	// new line
			char str[128];
			sprintf(str, "mPieceIndex = %u", m->mPieceIndex);
			details += str;
			details += "<BR>";
			sprintf(str, "mBegin = %u", m->mBegin);
			details += str;
			details += "<BR>";
			sprintf(str, "mLength = %u", m->mLength);
			details += str;
		}
			break;
		case TM_PORT:
		{
			details += "<B>";
			details += "PORT";
			details += "</B>";	// /bold
		}
			break;

		default:
			assert(0);
		}
		return details;
	}

	if(messageType == INCOMING_CONNECTION)
	{
		const cSockAddr* pSockAddr = reinterpret_cast<const cSockAddr*> (pMessage);
		details += "<B>";	// bold
		details += "Incoming Connection";
		details += "</B>";
		details += "<BR>";	// new line
		char str[128];
		sprintf(str, "Ip : %s Port %u", pSockAddr->Ip().AsString(), pSockAddr->Port());
		details += str;
		details += "<BR>";
		return details;
	}


	if(messageType == OUTGOING_CONNECTION)
	{
		const cSockAddr* pSockAddr = reinterpret_cast<const cSockAddr*> (pMessage);
		details += "<B>";	// bold
		details += "Outgoing Connection";
		details += "</B>";
		details += "<BR>";	// new line
		char str[128];
		sprintf(str, "Ip : %s Port %u", pSockAddr->Ip().AsString(), pSockAddr->Port());
		details += str;
		return details;
	}

	if(messageType == HANDSHAKE)
	{
		const cTorrentHandshakeMessage* message = reinterpret_cast<const cTorrentHandshakeMessage*> (pMessage);
		
		details += "<B>";	// bold
		details += "HANDSHAKE";
		details += "</B>";
		details += "<BR>";	// new line
		char str[128];
		sprintf(str, "UID ");
		
		//char sz[32];
		//sprintf("%X%X%X%X%X\n", static_cast<u32>(message->mPeerId[0]), static_cast<u32>(message->mPeerId[4]), static_cast<u32>(mPeerId[8]), static_cast<u32>(message->mPeerId[12]), static_cast<u32>(message->mPeerId[16]));
	
		char sz[32];
		memset(sz, 0, sizeof(sz));
		for(u32 i=0; i < PEER_ID_LENGTH; i++)
		{			
			sprintf(sz, "%c", message->mPeerId[i]);			
			strcat(str, sz);
		}
		
		details += str;
		return details;
	}


	if(messageType == CLOSE_CONNECTION)
	{
		const cSockAddr* pSockAddr = reinterpret_cast<const cSockAddr*> (pMessage);
		details += "<B>";	// bold
		details += "Close Connection";
		details += "</B>";
		details += "<BR>";	// new line
		char str[128];
		sprintf(str, "Ip : %s Port %u", pSockAddr->Ip().AsString(), pSockAddr->Port());
		details += str;
		return details;
	}
		
	if(messageType == BAD_MESSAGE_CLOSE_CONNECTION)
	{
		const char* szErr = reinterpret_cast<const char*> (pMessage);
		details += "<B>";	// bold
		details += "BAD MESSAGE RECEIVED";
		details += "</B>";
		details += "<BR>";	// new line
		details += szErr;
		return details;
	}

	assert(0);
	return details;
}// END FormatCellForPacket