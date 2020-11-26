// Jon Bellamy 26-01-2008


#if USE_PCH
#include "stdafx.h"
#endif

#include "Network/NetSettings.h"
#include "Network/Ports.h"
#include "HttpHeader.h"

#include <assert.h>


#define USER_AGENT_NAME "JBNETLIB/1.0"

namespace net 
{



cHttpMessageHeader::cHttpMessageHeader()
: mReadOffset(0)
, mHeaderData(DEAFAULT_HEADER_BUFFER_SIZE)
{
	Clear();
}



void cHttpMessageHeader::Clear()
{
	mReadOffset = 0;
	mHeaderData.Clear();
	mRecipientUrl.Clear();
}// END Clear



void cHttpMessageHeader::Write(u8* headerData, u32 size)
{
	mHeaderData.StreamBytes(headerData, size);
}// END Write



void cHttpMessageHeader::WriteRequest(const std::string& command, const net::cUrl& url, bool bAddClose)
{
	WriteStandardHeaderParts(command, url);

	std::string header;	
	if(bAddClose)
	{
		header += "Connection: close\r\n";
	}
	
	header += "\r\n";

	mHeaderData.StreamBytes(reinterpret_cast<const u8*>(header.c_str()), static_cast<u32>(header.size()));

#if HTTP_DEBUG_MESSAGES
	Printf("%s", header.c_str());
#endif
}// END WriteRequest



// we have the POST form from here, http://www.jmarshall.com/easy/http/#postmethod

// POST /path/script.cgi HTTP/1.0
// From: frog@jmarshall.com
// User-Agent: HTTPTool/1.0
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 32

// Home=Cosby&favorite+flavor=flies

// However the Gamespy POST command sends in a different form of:

// POST /rockstarWebServices/GS/Login.aspx?email=steve.wilson%40rockstarleeds.com&pass=MA203DT&platform=6&title=6 HTTP/1.1
// Host: socialclub.rockstargames.com:443
// User-Agent: GameSpyHTTP/1.0
// Connection: close
// Content-Length: 0
// Content-Type: application/x-www-form-urlencoded

// both methods seem to work but i have gone with the first style which sends the params in the message body rather than in the resource path
void cHttpMessageHeader::WritePost(const std::string& command, const net::cUrl& url)
{
	std::string postParams = url.GetAllParams();

	WriteStandardHeaderParts(command, url);
	WriteContentLength(static_cast<u32>(postParams.size()));
	
	std::string header;
	header += "Connection: close\r\n";
	header += "Content-Type: application/x-www-form-urlencoded\r\n";
	
	// NB : the double new line means that the params are in fact the message body not the header
	header += "\r\n";

	// add the data we are sending 
	header += postParams + "\r\n";

	// end of message body
	header += "\r\n";

	mHeaderData.StreamBytes(reinterpret_cast<const u8*>(header.c_str()), static_cast<u32>(header.size()));
}// END WritePost



void cHttpMessageHeader::WriteSoapRequest(const std::string& command, const net::cUrl& url, const std::string& soapActionTag, u32 contentLength)
{
	std::string header;

	WriteStandardHeaderParts(command, url);
	WriteContentLength(contentLength);
	
	// 1.2 i think
	//header += "Content-Type: application/soap+xml; charset=utf-8\r\n";

	// 1.1
	header += "Content-Type: text/xml; charset=utf-8\r\n";
	
	// this is a Soap1.1 header, 1.2 adds it in the Content-Type tag:
	//		Content-Type: application/soap+xml;charset=UTF-8;action="http://localhost/samples/echo_service/echoString"

	// TODO : this is a dev namespace !!!
	//header += "SOAPAction: http://tempuri.org/\r\n";
	//header += "SOAPAction: http://terraserver-usa.com/terraserver/GetTile\r\n";
	if(soapActionTag != "")
	{
		header += "SOAPAction: " + soapActionTag + "\r\n";
	}
	else
	{
		header += "SOAPAction: \"\"\r\n";
	}
	

	// end of message body
	header += "\r\n";

	mHeaderData.StreamBytes(reinterpret_cast<const u8*>(header.c_str()), static_cast<u32>(header.size()));

	// soap xml now follows in message body...
}// END WriteSoapRequest



void cHttpMessageHeader::WriteStandardHeaderParts(const std::string& command, const net::cUrl& url)
{
	Clear();
	std::string header;

	header = command + " HTTP/1.1" + "\r\n";
	header += ("Host: " + url.HostName());

	// always going to add the port here, not sure if this is ok but i can't really see it causing problems
	char szPort[16];
	u16 port;
	if(url.HasPort())
	{
		port = url.Port();
	}
	else
	{
		switch(url.Scheme())
		{
		case cUrl::SCHEME_HTTP:
			port = HTTP_PORT;
			break;

		case cUrl::SCHEME_HTTPS:
			port = HTTPS_PORT;
			break;

		default:
			assert(0);
			break;
		}
	}
	sprintf(szPort, "%u", port);
	header = header + ":" + szPort + "\r\n";
	 
	header = header + "User-Agent: " + USER_AGENT_NAME + "\r\n";

	mHeaderData.StreamBytes(reinterpret_cast<const u8*>(header.c_str()), static_cast<u32>(header.size()));
}// END WriteStandardHeaderParts



void cHttpMessageHeader::WriteContentLength(u32 length)
{
	char szNum[32];
	sprintf(szNum, "%u", length);
	std::string header;
	header = header + "Content-Length: " + szNum + "\r\n";
	mHeaderData.StreamBytes(reinterpret_cast<const u8*>(header.c_str()), static_cast<u32>(header.size()));
}// END WriteContentLength



bool cHttpMessageHeader::ContentLength(u32* pContentLengthOut) const
{
	const char szHeaderName[] = "Content-Length:";
	if(!PointAtTag(szHeaderName, sizeof(szHeaderName)-1))
	{
		*pContentLengthOut = 0;
		return false;
	}

	const char* pLine = CurrentLine();
	u32 length=0;
	s32 ret = sscanf_s(pLine+sizeof(szHeaderName)-1, "%d", &length, 16);
	*pContentLengthOut =  length;
	return true;
}// END ContentLength



bool cHttpMessageHeader::Date(std::string& strOut) const
{
	const char szHeaderName[] = "Date:";
	bool bTagFound = PointAtTag(szHeaderName, sizeof(szHeaderName)-1);

	if(!bTagFound)
	{
		return false;
	}

	strOut = GetTagValue();

	return true;
}// END Date



s32 cHttpMessageHeader::ReplyCode()
{
	std::string str = FirstLine();

	// all reply messages start with the http version followed by a space, the return code comes after the space
	// TODO : it feels like there would be a much better way to parse this

	u32 indexStart = static_cast<u32>(str.find(" "));
	indexStart++;
	u32 indexEnd = static_cast<u32>(str.find(" ", indexStart));

	std::string strValue;
	strValue = str.substr(indexStart, indexEnd - indexStart);

	s32 replyCode=-1;

	// note 3 byte return code
	s32 ret = sscanf_s(strValue.c_str(), "%d", &replyCode, 3);

	if(ret == EOF || ret != 1)
	{
		assert(0);
	}
	return replyCode;
}// END ReplyCode



HttpReplyCodeType cHttpMessageHeader::ReplyCodeType()
{
	s32 replyCode = ReplyCode();

	assert(replyCode >= 100);

	if(replyCode >= HTTP_REPLY_INFORMATIONAL_MESSAGE && replyCode <= s32(HTTP_REPLY_INFORMATIONAL_MESSAGE + 99))
	{
		return HTTP_REPLY_INFORMATIONAL_MESSAGE;
	}

	if(replyCode >= HTTP_REPLY_SUCCCESS && replyCode <= s32(HTTP_REPLY_SUCCCESS + 99))
	{
		return HTTP_REPLY_SUCCCESS;
	}

	if(replyCode >= HTTP_REPLY_REDIRECTION && replyCode <= s32(HTTP_REPLY_REDIRECTION + 99))
	{
		return HTTP_REPLY_REDIRECTION;
	}

	if(replyCode >= HTTP_REPLY_CLIENT_ERROR && replyCode <= s32(HTTP_REPLY_CLIENT_ERROR + 99))
	{
		return HTTP_REPLY_CLIENT_ERROR;
	}

	if(replyCode >= HTTP_REPLY_SERVER_ERROR && replyCode <= s32(HTTP_REPLY_SERVER_ERROR + 99))
	{
		return HTTP_REPLY_SERVER_ERROR;
	}


	assert(0);
	return HTTP_REPLY_UNKNOWN_ERROR;
}// END ReplyCodeType



bool cHttpMessageHeader::IsContentChunked() const
{
	std::string tranferEncoding;
	bool bTagFound = TransferEncoding(tranferEncoding);

	if(bTagFound && tranferEncoding == "chunked")
	{
		return true;
	}
	return false;
}// END IsContentChunked



// TODO : these functions should be replaced with one where you pass in the tag name !!!


bool cHttpMessageHeader::TransferEncoding(std::string& strOut) const
{
	const char szHeaderName[] = "Transfer-Encoding:";
	bool bTagFound = PointAtTag(szHeaderName, sizeof(szHeaderName)-1);

	if(!bTagFound)
	{
		return false;
	}
	
	strOut = GetTagValue();

	return true;
}// END TransferEncoding



bool cHttpMessageHeader::Location(std::string& strOut) const
{
	const char szHeaderName[] = "Location:";
	bool bTagFound = PointAtTag(szHeaderName, sizeof(szHeaderName)-1);

	if(!bTagFound)
	{
		return false;
	}

	strOut = GetTagValue();

	return true;
}// END Location


std::string cHttpMessageHeader::GetTagValue() const
{
	std::string str = CurrentLine();

	u32 indexStart = static_cast<u32>(str.find(":"));
	u32 indexEnd = static_cast<u32>(str.find("\r\n"));
	assert(indexStart != std::string::npos);

	std::string strValue;
	strValue = str.substr(indexStart+2, indexEnd - indexStart - 2);
	return strValue;
}// END GetTagValue



// tagLength must NOT include any NULL terminator
bool cHttpMessageHeader::PointAtTag(const char* tag, u32 tagLength) const
{
	ResetReadOffset();
	const char* pLine = FirstLine();

	// ensure to ignore the NULL terminator
	while(pLine)
	{
		if(memcmp(tag, pLine, tagLength) == 0)
		{
			return true;
		}
		pLine = NextLine();
	}

	return false;
}// END PointAtTag




const char* cHttpMessageHeader::FirstLine() const
{
	assert(mHeaderData.Size() > 0);
	return reinterpret_cast<const char*> (&mHeaderData[0]);
}// END FirstLine



const char* cHttpMessageHeader::CurrentLine() const
{
	assert(mHeaderData.Size() > 0);
	return reinterpret_cast<const char*> (&mHeaderData[mReadOffset]);
}// END CurrentLine


const char* cHttpMessageHeader::NextLine() const
{
	assert(mHeaderData.Size() > 0);
	assert(mReadOffset < mHeaderData.Size());

	while(mReadOffset < mHeaderData.Size()-2)
	{
		const char* debug = (char*) (&mHeaderData[mReadOffset]);

		if(mHeaderData[mReadOffset] == 13 &&
		   mHeaderData[mReadOffset+1] == 10)
		{
			mReadOffset += 2;
			return reinterpret_cast<const char*> (&mHeaderData[mReadOffset]);
		}
		mReadOffset++;
	}

	return NULL;
}// END NextLine



//////////////////////////////////////////////////////////////////////////
// Chunk Header


cHttpChunkHeader::cHttpChunkHeader()
: mHeaderSize(0)
{
	Clear();
}// END cHttpChunkHeader



void cHttpChunkHeader::Clear()
{
	memset(mHeaderData, 0xdd, MAX_HEADER_SIZE);
}// END Clear



void cHttpChunkHeader::Write(u8* source, u32 headerSize)
{
	memset(mHeaderData, 0, MAX_HEADER_SIZE);
	assert(headerSize < MAX_HEADER_SIZE);
	mHeaderSize = headerSize;
	memcpy(mHeaderData, source, headerSize);
}// END Write



// converts the hex digits
s32 cHttpChunkHeader::ChunkSize()
{
	s32 length;
	s32 ret = sscanf_s(reinterpret_cast<const char*>(&mHeaderData[0]), "%x", &length, mHeaderSize);

	if(ret == EOF || ret != 1)
	{
		return -1;
	}
	return length;
}// END ChunkSize



} // namespace net