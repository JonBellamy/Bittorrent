// Jon Bellamy 21-01-2008



#ifndef _HTTP_HEADER_H
#define _HTTP_HEADER_H


#include <string>
#include "Network/bytestream.h"
#include "Network/Url.h"

namespace net 
{


typedef enum
{
	HTTP_REPLY_INFORMATIONAL_MESSAGE = 100,			// 1XX
	HTTP_REPLY_SUCCCESS = 200,						// 2XX
	HTTP_REPLY_REDIRECTION = 300,					// 3XX
	HTTP_REPLY_CLIENT_ERROR = 400,					// 4XX
	HTTP_REPLY_SERVER_ERROR = 500,					// 5XX

	HTTP_REPLY_UNKNOWN_ERROR = 999
}HttpReplyCodeType;



class cHttpMessageHeader
{
public:	
	cHttpMessageHeader();

	const char* Header() const { return reinterpret_cast<const char *> (mHeaderData.Data()); }
	u32 Size() const { return mHeaderData.Size(); }

	void Write(u8* headerData, u32 size);
	
	void WriteRequest(const std::string& command, const net::cUrl& url, bool bAddClose);
	void WritePost(const std::string& command, const net::cUrl& url);
	void WriteSoapRequest(const std::string& command, const net::cUrl& url, const std::string& soapActionTag, u32 contentLength);

	void WriteContentLength(u32 length);

	void SetRecipient(const net::cUrl& url) { mRecipientUrl = url; }
	const net::cUrl& Recipient() const { return mRecipientUrl; }

	// TODO : IMPORTANT !!!
	//bool IsReply();
	
	s32 ReplyCode();
	HttpReplyCodeType ReplyCodeType();

	bool IsContentChunked() const;

	bool ContentLength(u32* pContentLengthOut) const;

	bool Date(std::string& strOut) const;

	bool TransferEncoding(std::string& strOut) const;
	
	bool Location(std::string& strOut) const;

	void Clear();

protected:

	void WriteStandardHeaderParts(const std::string& command, const net::cUrl& url);

	// tagLength must NOT include any NULL terminator
	bool PointAtTag(const char* tag, u32 tagLength) const;
	std::string GetTagValue() const;

	void ResetReadOffset() const { mReadOffset=0; }
	const char* FirstLine() const;
	const char* CurrentLine() const;
	const char* NextLine() const;

	enum
	{
		DEAFAULT_HEADER_BUFFER_SIZE = 1024
	};

private:
	cByteStream mHeaderData;

	mutable u32 mReadOffset;

	net::cUrl mRecipientUrl;
};



// Some http requests have their data returned as a series of data chunks (generally when the data is being generated 
// by the http server dynamically and so the full size is not known at the start). This header is used to parse the chunks.
// Chunk headers are a string of ascii encoded hex digits that denote the chunk size
class cHttpChunkHeader
{
public:
	cHttpChunkHeader();
	~cHttpChunkHeader() {}

	void Clear();

	void Write(u8* source, u32 headerSize);

	// converts the hex digits
	s32 ChunkSize();

private:

	enum
	{
		MAX_HEADER_SIZE = 16
	};

	u8 mHeaderData[MAX_HEADER_SIZE];
	u32 mHeaderSize;
};


} // namespace net


#endif // _HTTP_HEADER_H