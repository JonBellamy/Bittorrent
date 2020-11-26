// Jon Bellamy 26-01-2008


#if USE_PCH
#include "stdafx.h"
#endif

#include "HttpClient.h"

#include <string>

#include "Network/soap/Soap.h"
#include "Network/Dns.h"
#include "Network/Ports.h"
//#include "cassert.h"
#include "File/file.h"


namespace net 
{



cHttpClient::cHttpClient(u32 recvBufferSize)
: mState(STATE_IDLE)
, mTimer(cTimer::STOPWATCH)
, mConnectTimeout(DEFAULT_CONNECT_TIMEOUT)
, mpSocket(NULL)
, mRecvBuffer(recvBufferSize)
, mStreamType(STREAM_CONTENT_ERR)
, mHeadRcvdCb(NULL)
, mContentRcvdCb(NULL)
, mCommandSent(CMD_INVALID)
, mHeadCbParam(NULL)
, mContentCbParam(NULL)
, mCurrentChunkBytesRecvd(0)
, mRedirecting(false)
{
	mTimer.Start();
}// END cHttpClient



cHttpClient::~cHttpClient()
{
	Close();
}// END ~cHttpClient



bool cHttpClient::Connect(const cUrl& url)
{
	assert(mpSocket==NULL);

	mRecvBuffer.Clear();
	WinSock().Open();

	bool bRet;
	cIpAddr ipAddr; 

	bRet = cDns::IpAddressFromDomainName(url.HostName().c_str(), 0, &ipAddr);

	//assert(bRet); //, "failed to lookup name from dns");
	if(!bRet)
	{
		return false;
	}

	u16 httpPort;
	switch(url.Scheme())
	{
	case cUrl::SCHEME_HTTP:
	case cUrl::SCHEME_NOT_SPECIFIED:
		httpPort = HTTP_PORT;
		mpSocket = new cTcpSocket;
		break;

	case cUrl::SCHEME_HTTPS:
		httpPort = HTTPS_PORT;
		mpSocket = new cSslSocket;
		break;

	default:
		mpSocket=NULL;
		Close();
#if HTTP_DEBUG_MESSAGES
		Printf("HttpClient: Unsupported http url %s\n", url.AsString());
#endif
		return false;
	}

	u16 port = url.HasPort() ? url.Port() : httpPort;
	cSockAddr addr(ipAddr, port);

	bRet = mpSocket->OpenAndConnect(addr, false);
	if(!bRet)
	{
		return false;
	}
	//assert(bRet); //, "connection failed");

	mTimer.Process();
	mRequestTime = mTimer.ElapsedMs();

	return true;
}// END Connect



void cHttpClient::Close()
{
	if(mpSocket)
	{
		mpSocket->Close();
		delete mpSocket;
		mpSocket=NULL;
	}

	mCommandSent = CMD_INVALID;
	mState = STATE_IDLE;
	mRedirecting = false;

	mRequestTime = 0;
	mRecvBuffer.Clear();
	mSendHeader.Clear();
	mSendContent.Clear();
	mReplyMessage.Clear();
	mChunkHeader.Clear();
	mCurrentChunkBytesRecvd = 0;
	mRedirecting = false;
}// END Close



bool cHttpClient::GET(const cUrl& url, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam, HttpRequestCompleteCb contentRecvbCb, void* contentCbParam)
{
	assert(!mpSocket || !mpSocket->IsOpen()); //, "socket open, need to support persistent connections but not done it yet!");
	
	if(IsProcessingRequest())
	{
		return false;
	}

	mStreamType = streamType;

	mHeadRcvdCb = headRecvbCb;
	mContentRcvdCb = contentRecvbCb;
	mHeadCbParam = headCbParam;
	mContentCbParam = contentCbParam;
	mUrl = url;
	mReplyMessage.MessageHeader().Clear();

	if (!Connect(url))
	{
		Printf("Failed to connect to %s\n", mUrl.HostName().c_str());
		return false; 
	}

	mSendHeader.Clear();
	mSendContent.Clear();
	mSendHeader.WriteRequest(std::string("GET " + mUrl.ResourceWithPath()), mUrl, true);
	mSendHeader.SetRecipient(mUrl);
	mRecvBuffer.Clear();
	mCommandSent = CMD_GET;

#if HTTP_DEBUG_MESSAGES
	Printf("Sending HTTP Request:\n");
	Printf("%s\n", mSendHeader.Header());
#endif

	//mpSocket->Send(header.Header(), header.Size());
	//mState = STATE_WAITING_FOR_REPLY_HEADER;
	mState = STATE_CONNECTING_TO_SERVER;

	return true;
}// END GET



// Asks for the response identical to the one that would correspond to a GET request, but without the response body. This is useful for retrieving meta-information written in response headers, without having to transport the entire content.
bool cHttpClient::HEAD(const cUrl& url, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam)
{
	assert(!mpSocket || !mpSocket->IsOpen()); //, "socket open, need to support persistent connections but not done it yet!");

	if(IsProcessingRequest())
	{
		return false;
	}

	mHeadRcvdCb = headRecvbCb;
	mHeadCbParam = headCbParam;
	mUrl = url;
	mStreamType = streamType;
	mReplyMessage.MessageHeader().Clear();

	if (!Connect(url))
	{
		Printf("Failed to connect to %s\n", mUrl.HostName().c_str());
		return false; 
	}

	mSendHeader.Clear();
	mSendContent.Clear();	
	mSendHeader.WriteRequest(std::string("HEAD " + mUrl.ResourceWithPath()), mUrl, true);
	mSendHeader.SetRecipient(mUrl);
	mRecvBuffer.Clear();
	mCommandSent = CMD_HEAD;

#if HTTP_DEBUG_MESSAGES
	Printf("Sending HTTP Request:\n");
	Printf(mSendHeader.Header());
#endif

	//mpSocket->Send(mSendHeader.Header(), mSendHeader.Size());
	//mState = STATE_WAITING_FOR_REPLY_HEADER;
	mState = STATE_CONNECTING_TO_SERVER;
	
	return true;
}// END HEAD



bool cHttpClient::POST(const cUrl& url, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam, HttpRequestCompleteCb contentRecvbCb, void* contentCbParam)
{
	assert(!mpSocket || !mpSocket->IsOpen()); //, "socket open, need to support persistent connections but not done it yet!");

	if(IsProcessingRequest())
	{
		return false;
	}

	mHeadRcvdCb = headRecvbCb;
	mContentRcvdCb = contentRecvbCb;
	mHeadCbParam = headCbParam;
	mContentCbParam = contentCbParam;
	mUrl = url;
	mStreamType = streamType;
	mReplyMessage.MessageHeader().Clear();


	if (!Connect(url))
	{
		Printf("Failed to connect to %s\n", mUrl.HostName().c_str());
		return false; 
	}

	mSendHeader.Clear();
	mSendContent.Clear();	
	mSendHeader.WritePost(std::string("POST " + mUrl.ResourceWithPath()), mUrl);
	mSendHeader.SetRecipient(mUrl);
	mRecvBuffer.Clear();
	mCommandSent = CMD_POST;

#if HTTP_DEBUG_MESSAGES
	Printf("Sending HTTP Request:\n");
	Printf(mSendHeader.Header());
#endif

	//mpSocket->Send(mSendHeader.Header(), mSendHeader.Size());
	//mState = STATE_WAITING_FOR_REPLY_HEADER;
	mState = STATE_CONNECTING_TO_SERVER;
	
	return true;
}// END POST



bool cHttpClient::SendSoapRequest(const cUrl& url, const cSoapRequest* pSoapRequest, const std::string& soapActionTag, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam, HttpRequestCompleteCb contentRecvbCb, void* contentCbParam)
{
	assert(!mpSocket || !mpSocket->IsOpen()); //, "socket open, need to support persistent connections but not done it yet!");

	if(IsProcessingRequest())
	{
		return false;
	}

	mHeadRcvdCb = headRecvbCb;
	mContentRcvdCb = contentRecvbCb;
	mHeadCbParam = headCbParam;
	mContentCbParam = contentCbParam;
	mUrl = url;
	mStreamType = streamType;
	mReplyMessage.MessageHeader().Clear();


	if (!Connect(url))
	{
		Printf("Failed to connect to %s\n", mUrl.HostName().c_str());
		return false; 
	}

	mSendHeader.Clear();
	mSendContent.Clear();	

	// append the new lines to end the body
	std::string requestXml = pSoapRequest->AsString();
	requestXml += "\r\n\r\n";


	mSendHeader.WriteSoapRequest(std::string("POST " + url.ResourceWithPath()), url, soapActionTag, static_cast<u32>(requestXml.size()));
	mSendHeader.SetRecipient(mUrl);
	mRecvBuffer.Clear();
	mCommandSent = CMD_POST_SOAP;

#if HTTP_DEBUG_MESSAGES
	Printf("Sending HTTP Request:\n");
	Printf(mSendHeader.Header());
	Printf("%s\n", requestXml.c_str());
#endif
	//mpSocket->Send(mSendHeader.Header(), header.Size());


	mSendContent.StreamBytes(reinterpret_cast<const u8*>(requestXml.c_str()), static_cast<u32>(requestXml.size()));
	//mpSocket->Send(requestXml.c_str(), static_cast<u32>(requestXml.size()));
	//mState = STATE_WAITING_FOR_REPLY_HEADER;
	mState = STATE_CONNECTING_TO_SERVER;

	return true;
}// END SendSoapRequest



void cHttpClient::Process()
{
	mTimer.Process();

	switch(mState)
	{
	case STATE_IDLE:
		break;

	case STATE_CONNECTING_TO_SERVER:
		ProcessState_Connecting();
		break;

	case STATE_WAITING_FOR_REPLY_HEADER:
		ProcessState_WaitingForReplyHeader();
		break;

	case STATE_WAITING_FOR_CONTENT_NORMAL:
		ProcessState_WaitingForContentNormal();
		break;


	case STATE_WAITING_FOR_CHUNK_HEADER:		
		ProcessState_WaitingForChunkHeader();
		break;


	case STATE_WAITING_FOR_CHUNK:
		ProcessState_WaitingForChunkData();
		break;

	default:
		assert(0);
		break;
	}
}// END Process



void cHttpClient::ProcessState_Connecting()
{
	assert(mpSocket->IsOpen());
	if(mpSocket->ConnectionEstablished())
	{
		mpSocket->Send(mSendHeader.Header(), mSendHeader.Size());
		if(mSendContent.Size() > 0)
		{
			mpSocket->Send(mSendContent.Data(), mSendContent.Size());
		}
		mState = STATE_WAITING_FOR_REPLY_HEADER;
	}
	else
	{
		if(mTimer.ElapsedMs() - mRequestTime >= mConnectTimeout)
		{
#if HTTP_DEBUG_MESSAGES
			Printf("Http Connect Failed (timeout)\n");
#endif
			
			if(mHeadRcvdCb)
			{
				mHeadRcvdCb(false, mSendHeader, mReplyMessage, mHeadCbParam);
			}

			if(mContentRcvdCb)
			{
				mContentRcvdCb(false, mSendHeader, mReplyMessage, mContentCbParam);
			}

			Close();
		}
	}
}// END ProcessState_Connecting



void cHttpClient::ProcessState_WaitingForReplyHeader()
{
	bool gotHeader = ReceiveReplyHeader();

	if(!IsProcessingRequest())
	{
		// something has stopped us, bail
		return;
	}

	if(gotHeader)
	{
		u32 replyCode = mReplyMessage.MessageHeader().ReplyCode();

		// check the reply type and bail if need be 
		if(mReplyMessage.MessageHeader().ReplyCodeType() != HTTP_REPLY_SUCCCESS)
		{
			switch(mReplyMessage.MessageHeader().ReplyCodeType())
			{
			case HTTP_REPLY_REDIRECTION:
				HandleRedirection(replyCode);
				return;

			default:
				// If there is an error but there is still content to get (error reply etc) then continue and get it. Request will then complete
				// successfully but it will be an error description in the content, callers duty to check the reply code
				u32 contentLength;
				bool ret = mReplyMessage.MessageHeader().ContentLength(&contentLength);
				if(!ret || contentLength == 0)
				{
					RequestCannotBeFulFilled(replyCode);
					return;
				}
				break;
			}

			mRedirecting=false;
		}


		mRedirecting = false;

		// got the head ...

		if(mHeadRcvdCb)
		{
			mHeadRcvdCb(true, mSendHeader, mReplyMessage, mHeadCbParam);
		}

		if(mCommandSent == CMD_HEAD)
		{
			mState = STATE_IDLE;
			Close();
			return;
		}


		// start receiving the content
		if(mReplyMessage.MessageHeader().IsContentChunked())
		{
			mRecvBuffer.Clear();

			mChunkHeader.Clear();

			if(!mReplyMessage.BeginStreamingContent(mUrl, 0, mStreamType))
			{
				Close();
				return;
			}

			// TODO : check for if there are trailers here, otherwise its gonna crash when there are some
			// trailers: t1

			mState = STATE_WAITING_FOR_CHUNK_HEADER;
		}
		else	
		{
			mRecvBuffer.Clear();

			u32 contentLength;
			bool ret = mReplyMessage.MessageHeader().ContentLength(&contentLength);
			if(!ret || !mReplyMessage.BeginStreamingContent(mUrl, contentLength, mStreamType))
			{
#if HTTP_DEBUG_MESSAGES
				Printf("Failed to stream http content, aborting\n");
#endif
				Close();
				return;
			}
			mState = STATE_WAITING_FOR_CONTENT_NORMAL;
		}
	}
}// END ProcessState_WaitingForReplyHeader



void cHttpClient::ProcessState_WaitingForContentNormal()
{
	s32 bytesRcvd = ReceiveReplyContent();
	if(bytesRcvd < 0)
	{
		Close();
		return;
	}
	if(bytesRcvd > 0)
	{
		if(mReplyMessage.FinishedStreamingContent())
		{

#if HTTP_DEBUG_MESSAGES
			Printf("\nHttp request complete\n");
#endif

			mState = STATE_IDLE;

			if(mContentRcvdCb)
			{
				mContentRcvdCb(true, mSendHeader, mReplyMessage, mContentCbParam);
			}

			Close();
		}
	}
}// END ProcessState_WaitingForContentNormal



void cHttpClient::ProcessState_WaitingForChunkHeader()
{
	bool gotHeader = ReceiveChunkHeader();

	if(!IsProcessingRequest())
	{
		// something has stopped us, bail
		return;
	}

	if(gotHeader)
	{
		s32 chunkSize = mChunkHeader.ChunkSize();

		// is chunk size 0, quit
		if(chunkSize==0)
		{
			// final chunk has a cr lf at the end, discard it. Shouldn't matter if this fails as we have the data and we are closing now
			ReadRedundantBytes(2);

			mReplyMessage.ContentStreamingFinished();

			mState = STATE_IDLE;

			if(mContentRcvdCb)
			{
				mContentRcvdCb(true, mSendHeader, mReplyMessage, mContentCbParam);
			}

			Close();
			mChunkHeader.Clear();

			return;
		}

		mCurrentChunkBytesRecvd=0;

		// receive chunk state
		mState = STATE_WAITING_FOR_CHUNK;
	}
}// END ProcessState_WaitingForChunkHeader



void cHttpClient::ProcessState_WaitingForChunkData()
{
	s32 chunkSize = mChunkHeader.ChunkSize();
	if(mCurrentChunkBytesRecvd == chunkSize)
	{
		// chunks are also terminated with a \cr \lf which is NOT part of the chunk data
		// read and discard these 2 bytes

		if (ReadRedundantBytes(2))
		{
#if HTTP_DEBUG_MESSAGES
//			Printf("Finished streaming chunk\n");					
#endif
			mCurrentChunkBytesRecvd =0;
			mChunkHeader.Clear();
			mState = STATE_WAITING_FOR_CHUNK_HEADER;
		}
		else
		{
			if(!IsProcessingRequest())
			{
				// something has stopped us, bail
				return;
			}
		}
	}
	else
	{
		// receive the current chunk data, but be careful not to receive data from the next chunk
		u32 bytesRecvd = ReceiveReplyContent(chunkSize - mCurrentChunkBytesRecvd);
		if(bytesRecvd < 0)
		{
			Close();
			return;
		}
		else
		{
			mCurrentChunkBytesRecvd += bytesRecvd;
		}
	}
}// END ProcessState_WaitingForChunkData




void cHttpClient::HandleRedirection(u32 replyCode)
{
	// we are not going to redirect twice
	if(mRedirecting)
	{
#if HTTP_DEBUG_MESSAGES
		Printf("\nRedirecting twice not supported to prevent infinite loops\n");
#endif

		if(mHeadRcvdCb)
		{
			mHeadRcvdCb(false, mSendHeader, mReplyMessage, mHeadCbParam);
		}

		Close();

		return;
	}

	const u32 MOVED_PERMANENTLY = 301;
	const u32 MOVED_TEMPORARILY = 302;
	if( replyCode == MOVED_PERMANENTLY ||
		replyCode == MOVED_TEMPORARILY)
	{
		std::string newLocation;

		if(mReplyMessage.MessageHeader().Location(newLocation))
		{
#if HTTP_DEBUG_MESSAGES
			Printf("\nRedirecting to %s\n", newLocation.c_str());
#endif

			HttpCommand cmd = mCommandSent;
			Close();
			mReplyMessage.MessageHeader().Clear();

			switch(cmd)
			{
			case CMD_HEAD:
				mRedirecting = HEAD(cUrl(newLocation.c_str()), mStreamType, mHeadRcvdCb, mHeadCbParam);
				break;

			case CMD_GET:
				mRedirecting = GET(cUrl(newLocation.c_str()), mStreamType, mHeadRcvdCb, mHeadCbParam, mContentRcvdCb, mContentCbParam);
				break;

			default:
				assert(0);
			}
		}
		else
		{
#if HTTP_DEBUG_MESSAGES
			Printf("No redirection 'location' tag\n");
#endif
			mRedirecting=false;

			assert(0);
		}
	}
}// END HandleRedirection




void cHttpClient::RequestCannotBeFulFilled(u32 replyCode)
{
#if HTTP_DEBUG_MESSAGES
	Printf("\nHttp request not fulfilled, reply code %d\n", mReplyMessage.MessageHeader().ReplyCode());
#endif
	Close();

	if(mHeadRcvdCb)
	{
		mHeadRcvdCb(false, mSendHeader, mReplyMessage, mHeadCbParam);
	}
}// END RequestCannotBeFulFilled



bool cHttpClient::ReadRedundantBytes(u32 numBytes)
{
	assert(numBytes > 0 && numBytes < mRecvBuffer.FreeSpace());

	if(mpSocket->BytesPendingOnInputBuffer() >= numBytes)
	{
		mRecvBuffer.Clear();
		int bytesRecvd = mpSocket->Recv(&mRecvBuffer, numBytes, false);
		if(bytesRecvd < 0)
		{
			Close();
			return false;
		}	
		assert(bytesRecvd == numBytes);
		mRecvBuffer.Clear();
		return true;
	}
	return false;
}// END ReadRedundantBytes



// chunk headers are terminated with a \cr \lf
bool cHttpClient::ReceiveChunkHeader()
{
	s32 bytesRecvd = mpSocket->Recv(&mRecvBuffer, mRecvBuffer.FreeSpace(), true);
	if(bytesRecvd < 0)
	{
		Close();
		return false;
	}

	if(bytesRecvd > 0)
	{
		for(s32 i=0; i < bytesRecvd; i++)
		{
			if(i >= 1)
			{
				// 1 empty lines denote the end of the header 
				if(mRecvBuffer[i - 1] == 13 && mRecvBuffer[i] == 10)
				{
					mRecvBuffer.Clear();

					mpSocket->Recv(&mRecvBuffer, i+1, false);

					mChunkHeader.Write(mRecvBuffer.Data(), i);		

					mRecvBuffer.Clear();

					return true;
				}
			}
		}
	}

	return false;
}// END ReceiveChunkHeader



bool cHttpClient::ReceiveReplyHeader()
{
	s32 bytesRecvd = mpSocket->Recv(&mRecvBuffer, mRecvBuffer.Capacity()/*mRecvBuffer.FreeSpace()*/, true);
	if(bytesRecvd < 0)
	{
		Close();
		return false;
	}

	if(bytesRecvd > 0)
	{
		for(s32 i=0; i < bytesRecvd; i++)
		{
			if(i >= 3)
			{
				// 2 empty lines denote the end of the header 
				if( mRecvBuffer[i - 3] == 13 && mRecvBuffer[i - 2] == 10 &&
					mRecvBuffer[i - 1] == 13 && mRecvBuffer[i] == 10)
				{
					mRecvBuffer.Clear();
					mpSocket->Recv(&mRecvBuffer, i+1, false);

					mReplyMessage.MessageHeader().Write(mRecvBuffer.Data(), i);		
					mRecvBuffer.Clear();
					
#if HTTP_DEBUG_MESSAGES
					Printf("HTTP reply header received:\n");
					Printf("%s\n", mReplyMessage.MessageHeader().Header());
#endif
					return true;
				}
			}
		}
	}

	return false;
}// END ReceiveReplyHeader



s32 cHttpClient::ReceiveReplyContent(u32 maxBytes)
{	
	// the recv buffer should be empty as we clear it after we stream data below
	assert(mRecvBuffer.Size() == 0);

	s32 bytesRecvd = mpSocket->Recv(&mRecvBuffer, min(maxBytes, mRecvBuffer.FreeSpace()));
	assert(bytesRecvd <= RECV_BUFFER_SIZE);
	if(bytesRecvd > 0)
	{
		mReplyMessage.StreamContent(mRecvBuffer.Data(), bytesRecvd);		
		mRecvBuffer.Clear();
	}
	return bytesRecvd;
}// END ReceiveReplyContent




// DEBUG
void cHttpClient::BlockForReplyHeader()
{
	while(!ReceiveReplyHeader());
}// END BlockForReplyHeader



// DEBUG
void cHttpClient::BlockForReplyContent()
{
	while(!ReceiveReplyContent());
}// END BlockForReplyContent



} // namespace net