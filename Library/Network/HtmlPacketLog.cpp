// html table to show the flow of packets


#if USE_PCH
#include "stdafx.h"
#endif


#include "HtmlPacketLog.h"


#include <string>

#include "File/FileHelpers.h"



cHtmlPacketLog::cHtmlPacketLog()
: cFile()
, mWritePosition(0)
, mPacketsLogged(0)
{
}// END cHtmlPacketLog



cHtmlPacketLog::~cHtmlPacketLog()
{
}// END ~cHtmlPacketLog



void cHtmlPacketLog::Init(const std::string& logFolder)
{
	mLogFolder = logFolder;
	CreateLogFolder(mLogFolder + "\\");
}// END Init



void cHtmlPacketLog::CreateLogFolder(const std::string& logFolder)
{
	bool ret = FileHelpers::MakeAllDirs(logFolder.c_str());
}// END CreateLogFolder



// clear temp folder, be VERY careful here
void cHtmlPacketLog::ClearLogFolder(const std::string& logFolder)
{
	FileHelpers::DeleteFolderContents(logFolder.c_str());
	CreateLogFolder(logFolder + "\\");
}// END ClearLogFolder



void cHtmlPacketLog::Open(const char *szFileName)
{
	char fn[512];
	strcpy(fn, mLogFolder.c_str());
	strcat(fn, "\\");
	strcat(fn, szFileName);
	if (!cFile::Open(cFile::WRITE, fn, false))
	{
		Printf("WARNING: Unable to open log file for writing. %s", szFileName);
		return;
	}

	std::string buf = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"> \r\n\
<HTML> \r\n\
	<HEAD> \r\n\
		<TITLE> Packet Log </TITLE> \r\n\
		<META NAME=\"Author - Jon Bellamy\" CONTENT=\"\"> \r\n\
		<META NAME=\"Description - Packet Log\" CONTENT=\"\"> \r\n\
	</HEAD> \r\n\
	<BODY> \r\n\
		<h2><center><i>Packet Log</center></i></h2> \r\n\
		<h3><left>Key<BR></left></h3> \r\n\
		<ul> \r\n\
			<li> <p style=\"color:black; font-size:14px;\">White = In order packet</p>  </li> \r\n\
			<li> <p style=\"color:orange; font-size:14px;\">Orange = Retransmitted packet</p>  </li> \r\n\
			<li> <p style=\"color:green; font-size:14px;\">Green = Out of order packet</p>  </li> \r\n\
			<li> <p style=\"color:gray; font-size:14px;\">Gray = Duplicate packet</p>  </li> \r\n\
			<li> <p style=\"color:red; font-size:14px;\">Red = Corrupt packet</p>  </li> \r\n\
		</ul> \r\n\
		<FONT FACE=\"Courier New\"> \r\n\
		<table border=\"1\" cellspacing=\"25\" cellpadding=\"5%\" width=\"auto\" style=\"font-size:12px;\"> \r\n\
			<tr> \r\n\
				<th>Packets Sent</th> \r\n\
				<th>Packets Received</th> \r\n\
				<th>Connection Info</th> \r\n\
			</tr> \r\n\
	";


	Write(-1, static_cast<u32>(buf.size()), reinterpret_cast<const u8*>(buf.c_str()));
	Flush();

	mWritePosition = static_cast<u32>(buf.find("</tr>"));
	assert(mWritePosition != std::string::npos);
	mWritePosition += 5;
}// END Open



// writes the closing tags to the end of the file then closes it
void cHtmlPacketLog::Close()
{	
	std::string buf = " \
		</table> \r\n\
		</FONT> \r\n \
	</BODY> \r\n\
</HTML> \
	";

	Write(-1, static_cast<u32>(buf.size()), reinterpret_cast<const u8*>(buf.c_str()));

	cFile::Close();
}// END Close


/* THIS RUDP stuff needs copying to RudpPacketLog (see torrent & dht packetlog)
void cHtmlPacketLog::LogPacket(u32 time, const net::iNetPacket& packet, HtmlPacketLogPType packetType)
{
	using namespace net;

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


	html.insert(pos, packet.LogHtmlPacket(time, packetType));


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


	Write(mWritePosition, static_cast<u32>(html.size()), reinterpret_cast<const u8*>(html.c_str()));

	mWritePosition += static_cast<u32>(html.size());

	// TODO : don't want to lose data but this will thrash the disk
	Flush();

//	DumpPacketToDisk(time, packet, packetType);
}// END LogPacket
*/

/*
std::string cHtmlPacketLog::FormatCellForPacket(u32 time, const net::cReliableUdpPacket& packet, HtmlPacketLogPType packetType)
{
	using namespace net;

	std::string details;
	
	details += "Flags - ";
	details += "<B>";	// bold
	for(u32 i=0; i < NUM_RUDP_PACKET_FLAGS; i++)
	{
		if(packet.IsFlagSet(PacketFlag(1<<i)))
		{
			details += packet.PacketFlagAsString(PacketFlag(1<<i));		

			details += " ";
		}
	}
	details += "</B>";	// /bold
	details += "<BR>";	// new line

	char sz[512];

	sprintf(sz, "t = %d<BR>Seq No - %u<BR>Ack - %u<BR>Checksum - 0x%X<BR>Payload Size - %u<BR><a href=\"file://%s\">Packet Data</a>", time, packet.Header().sequenceNumber, packet.Header().ackNumber, packet.Header().checksum, packet.PayloadSize(), GetPacketFileName(packet, packetType).c_str());
	details += sz;

	if(packet.IsFlagSet(EAK))
	{
		// figure out which segment(s) got lost and resend now
		u32 numSegmentsEakd = (packet.Header().headerLength - sizeof(cReliableUdpPacket::RudpPacketHeader)) / sizeof(u16);
		const u16* eakSegmentNumbers = reinterpret_cast<const u16*> (packet.Packet() + sizeof(cReliableUdpPacket::RudpPacketHeader));

		details += "<BR>EAK sequence numbers :<BR>";
		for(u32 i=0; i < numSegmentsEakd; i++)
		{
			sprintf(sz, "%d<BR>", eakSegmentNumbers[i]);
			details += sz;
		}	
	}

	return details;
}// END FormatCellForPacket
*/


bool cHtmlPacketLog::IsSendPacket(HtmlPacketLogPType packetType)
{
	return  packetType == PTYPE_SEND || packetType == PTYPE_RETRANSMIT;
}// END IsSendPacket



std::string cHtmlPacketLog::GetPacketColour(HtmlPacketLogPType packetType) const
{
	switch(packetType)
	{
	case PTYPE_SEND:
		return "white";

	case PTYPE_RECEIVE:
		return "white";

	case PTYPE_RETRANSMIT:
		return "orange";

	case PTYPE_RECEIVE_OUT_OF_ORDER:
		return "green";

	case PTYPE_RECEIVE_DUPE:
		return "gray";

	case PTYPE_RECEIVE_CORRUPT:
		return "red";

	default:
		assert(0);
		return "white";
	}
}// END GetPacketColour
