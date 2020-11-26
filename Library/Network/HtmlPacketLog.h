#ifndef _HTML_PACKET_LOG_H
#define _HTML_PACKET_LOG_H


#include "Network/NetSettings.h"

#include <string>


#include "File/file.h"



typedef enum
{
	PTYPE_SEND=0,
	PTYPE_RETRANSMIT,
	PTYPE_RECEIVE,
	PTYPE_RECEIVE_DUPE,
	PTYPE_RECEIVE_OUT_OF_ORDER,
	PTYPE_RECEIVE_CORRUPT,
}HtmlPacketLogPType;



class cHtmlPacketLog : public cFile
{
public:

	cHtmlPacketLog();
	virtual ~cHtmlPacketLog();

	void Init(const std::string& logFolder);

	static void CreateLogFolder(const std::string& logFolder);
	static void ClearLogFolder(const std::string& logFolder);

	void Open(const char *szFileName);
	virtual void Close();

	static bool IsSendPacket(HtmlPacketLogPType packetType);


protected:
	std::string GetPacketColour(HtmlPacketLogPType packetType) const;


	u32 mPacketsLogged;
	u32 mWritePosition;

	std::string mLogFolder;
};




#endif // _HTML_PACKET_LOG_H