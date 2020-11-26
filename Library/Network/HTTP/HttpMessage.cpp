// Jon Bellamy 26-01-2008


#if USE_PCH
#include "stdafx.h"
#endif

#include "HttpMessage.h"

#include <assert.h>
#include <stdio.h>

#include "File/FileHelpers.h"


namespace net 
{



cHttpMessage::cHttpMessage()
: mBytesStreamed(0)
, mFinishedStreaming(false)
, mStreamType(STREAM_CONTENT_ERR)
{
}



void cHttpMessage::Clear()
{
	mHeader.Clear();
	mBytesStreamed = 0;
	mFinishedStreaming = true;
	mStreamType = STREAM_CONTENT_ERR;

	if(mStreamFile.IsOpen())
	{
		mStreamFile.Close();
	}
	mStreamBuffer.Clear();
}// END Clear 



bool cHttpMessage::BeginStreamingContent(const cUrl& url, u32 contentSize, ContentStreamingType streamType)
{	
	mStreamType = streamType;
	switch(mStreamType)
	{
	case STREAM_CONTENT_TO_FILE:
	{
		// place the file in the correct place in the cache
		std::string path = ResourceLocalPath(url);
		FileHelpers::MakeAllDirs(path.c_str());
		std::string fn = path + url.ResourceName();
		
		mStreamFile.Open(cFile::WRITE, fn.c_str());
		assert(mStreamFile.IsOpen());
		break;
	}
		

	case STREAM_CONTENT_TO_BUFFER:
		mStreamBuffer.ClearAndResize(contentSize, false);
		break;

	default:
		assert(0);
		return false;
	}

	mBytesStreamed=0;
	mFinishedStreaming=false;

	return true;
}// END BeginStreamingContent



bool cHttpMessage::StreamContent(u8* pData, u32 numBytes)
{
	mBytesStreamed += numBytes;
	
#if HTTP_DEBUG_MESSAGES
	//Printf("%d Bytes streamed\n", numBytes);
#endif

	switch(mStreamType)
	{
	case STREAM_CONTENT_TO_FILE:
		mStreamFile.Write(cFile::FILE_POS_CURRENT, numBytes, pData);
		break;

	case STREAM_CONTENT_TO_BUFFER:
		mStreamBuffer.StreamBytes(pData, numBytes);
		break;

	default:
		assert(0);
		return false;
	}



	if(MessageHeader().IsContentChunked()==false)
	{
		u32 contentLength;
		bool ret = MessageHeader().ContentLength(&contentLength);
		if(!ret)
		{
#if HTTP_DEBUG_MESSAGES
			Printf("Missing content length tag, aborting\n");
#endif
			assert(0);
			return false;
		}

		// Warning this has fired (on a 404 i think) and should probably be looked into
		//assert(mBytesStreamed <= contentLength);
		if(mBytesStreamed > contentLength)
		{
			Printf("HTTP: WARNING received more bytes than content length.");
		}

		if(mBytesStreamed >= contentLength)
		{
			ContentStreamingFinished();
			return true;
		}
	}

	return true;
}// END StreamContent



void cHttpMessage::ContentStreamingFinished()
{
	switch(mStreamType)
	{
	case STREAM_CONTENT_TO_FILE:
		mStreamFile.Close();
		break;

	case STREAM_CONTENT_TO_BUFFER:
		break;

	default:
		assert(0);
		return;
	}
	
	mFinishedStreaming=true;

#if HTTP_DEBUG_MESSAGES
//	Printf("Finished streaming file\n");
#endif	
}// END ContentStreamingFinished



// used to place the file in the correct place in the local cache
std::string cHttpMessage::ResourceLocalPath(const cUrl& url) const
{	
	std::string path = "./HttpCache/" + url.HostName() + url.ResPathOnly() + "/";

	// TODO : if invalid chars become an issue, replace them here with an '_', use crt isalnum()?

	return path;
}// END ResourceLocalPath



} // namespace net