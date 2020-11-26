// Jon Bellamy 21-01-2008



#ifndef _HTTP_MESSAGE_H
#define _HTTP_MESSAGE_H


#include "HttpHeader.h"
#include "Network/NetSettings.h"
#include "Network/Url.h"
#include "Network/ByteStream.h"
#include "File/File.h"

namespace net 
{


typedef enum
{
	STREAM_CONTENT_TO_FILE =0,
	STREAM_CONTENT_TO_BUFFER,
	STREAM_CONTENT_ERR
}ContentStreamingType;



class cHttpMessage
{
public:	
	cHttpMessage();

	void Clear();

	bool BeginStreamingContent(const cUrl& url, u32 contentSize, ContentStreamingType streamType);
	bool StreamContent(u8* pData, u32 numBytes);
	
	bool FinishedStreamingContent() const { return mFinishedStreaming; }

	void ContentStreamingFinished();


	cHttpMessageHeader& MessageHeader() { return mHeader; }
	const cHttpMessageHeader& MessageHeader() const { return mHeader; }

	const cByteStream& StreamedContentBuffer() const { return mStreamBuffer; }

	// used to place the file in the correct place in the local cache
	std::string ResourceLocalPath(const cUrl& url) const;

private:


	cHttpMessageHeader mHeader;
	u32 mBytesStreamed;
	bool mFinishedStreaming;
	ContentStreamingType mStreamType;
	
	// We will stream the content into one of these
	cFile mStreamFile;
	cByteStream mStreamBuffer;
};



} // namespace net


#endif // _HTTP_MESSAGE_H