// html table to show the flow of packets


#if USE_PCH
#include "stdafx.h"
#endif


#include "DhtPacketLog.h"

#include <string>
#include <stdio.h>
#include <assert.h>

#include "Network/BitTorrent/dht/Dht.h"
#include "Network/BitTorrent/BitTorrentManager.h"



cDhtPacketLog::cDhtPacketLog()
{
}// END cDhtPacketLog



cDhtPacketLog::~cDhtPacketLog()
{
}// END ~cDhtPacketLog



void cDhtPacketLog::LogKrpcMessage(u32 time, const net::cSockAddr& addr, const cKrpcMsg& message/*, MessageType messageType*/, HtmlPacketLogPType packetType)
{
	using namespace net;

	mPacketsLogged++;

	std::string html = " \r\n\
				<tr> \r\n\
					<td bgcolor=\"\" align=\"left\" width=\"auto\"></td> \r\n\
					<td bgcolor=\"\" align=\"left\" width=\"auto\"></td> \r\n\
					<td bgcolor=\"white\" align=\"left\" valign=\"top\" width=\"auto\"></td> \r\n\
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


	html.insert(pos, FormatCellForPacket(time, addr, message, packetType));


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

	bool bRet = Write(mWritePosition, static_cast<u32>(html.size()), reinterpret_cast<const u8*>(html.c_str()));
	assert(bRet);

	mWritePosition += static_cast<u32>(html.size());

	// TODO : don't want to lose data but this will thrash the disk
	Flush();

//	DumpPacketToDisk(time, packet, packetType);
}// END LogKrpcMessage



// some vile void ptr stuff going on here
std::string cDhtPacketLog::FormatCellForPacket(u32 time, const net::cSockAddr& addr, const cKrpcMsg& message, HtmlPacketLogPType packetType)
{
	using namespace net;

	std::string details;
	char buf[64];
	
	assert(packetType == PTYPE_SEND || packetType == PTYPE_RECEIVE);

	details += "<B>";	// bold
	details += message.MsgTypeAsString();
	details += "</B>";
	
	details += "<BR>";	// new line
	details += "Time: ";
	sprintf(buf, "%d", BitTorrentManager().DhtTaskManager().Dht().Time());
	details += buf;
	

	details += "<BR>";	
	if(packetType == PTYPE_SEND)
	{
		details += std::string("To: ");
	}
	else
	{
		details += std::string("From: ");
	}
	details += addr.AsString();


	details += "<BR>";
	details += std::string("Transaction Id: ");
	sprintf(buf, "0x%.2X", message.TransactionId());
	details += buf;

	const cKrpcResponse* pResponse = dynamic_cast<const cKrpcResponse*> (&message);
	const cKrpcPingQuery* pPing = dynamic_cast<const cKrpcPingQuery*> (&message);
	const cKrpcGetPeersQuery* pGetPeers = dynamic_cast<const cKrpcGetPeersQuery*> (&message);
	const cKrpcFindNodeQuery* pFindNode = dynamic_cast<const cKrpcFindNodeQuery*> (&message);
	const cKrpcAnnouncePeerQuery* pAnnounce = dynamic_cast<const cKrpcAnnouncePeerQuery*> (&message);


	if(pResponse)
	{
		const cKrpcQuery* query = BitTorrentManager().DhtTaskManager().Dht().GetOutstandingMessage(pResponse->TransactionId());
		if(query)
		{
			details += "<BR>";
			details += std::string("Response To: ") + query->MsgTypeAsString();
		}
	}
	else if (pGetPeers)
	{
		const cDhtGetPeersTask* pTask = static_cast<const cDhtGetPeersTask*> (pGetPeers->GetTask());

		char hash[128];
		memset(hash, 0, sizeof(hash));
		for(u32 i=0; i < cDhtNodeId::NODE_ID_SIZE; i++)
		{
			u8 ch = (u8) pTask->ResourceId().AsString()[i];
			sprintf(buf, "%.2X ", ch);
			strcat(hash, buf);
		}

		details += "<BR>";
		details += std::string("Info Hash: ") + hash;		
	}
	else if (pFindNode)
	{
		const cDhtFindNodeTask* pTask = static_cast<const cDhtFindNodeTask*> (pFindNode->GetTask());

		char hash[128];
		memset(hash, 0, sizeof(hash));
		for(u32 i=0; i < cDhtNodeId::NODE_ID_SIZE; i++)
		{
			u8 ch = (u8) pTask->SearchTarget().AsString()[i];
			sprintf(buf, "%.2X ", ch);
			strcat(hash, buf);
		}

		details += "<BR>";
		details += std::string("Node Id: ") + hash;	
	}
	else if (pAnnounce)
	{
		const cDhtAnnounceTask* pTask = static_cast<const cDhtAnnounceTask*> (pAnnounce->GetTask());
		
		char hash[128];
		memset(hash, 0, sizeof(hash));
		for(u32 i=0; i < cDhtNodeId::NODE_ID_SIZE; i++)
		{
			u8 ch = (u8) pTask->ResourceId().AsString()[i];
			sprintf(buf, "%.2X ", ch);
			strcat(hash, buf);
		}

		details += "<BR>";
		details += std::string("Info Hash: ") + hash;	
	}
	else if (pPing)
	{
	}
	else
	{
		// TODO : incoming queries end up here
		//assert(0);
	}
	return details;
}// END FormatCellForPacket