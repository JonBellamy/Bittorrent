// Jon Bellamy 21-01-2008



#ifndef _HTTP_H
#define _HTTP_H


#include "Network/WinSock.h"
#include "Network/TcpSocket.h"
#include "Network/SslSocket.h"
#include "Network/NetSettings.h"
#include "Network/http/HttpMessage.h"
#include "Network/Url.h"

#include "General/Timer.h"


namespace net 
{


class cSoapRequest;


class cHttpClient
{
public:	
	cHttpClient(u32 recvBufferSize = RECV_BUFFER_SIZE);
	virtual ~cHttpClient();

	// Requests can be cancelled by closing
	void Close();


	typedef void (*HttpRequestCompleteCb) (bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param);

	// Requests a representation of the specified resource. By far the most common method used on the Web today.
	bool GET(const cUrl& url, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam, HttpRequestCompleteCb contentRecvbCb, void* contentCbParam);

	// Asks for the response identical to the one that would correspond to a GET request, but without the response body. This is useful for retrieving meta-information written in response headers, without having to transport the entire content.
	bool HEAD(const cUrl& url, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam);

	// A POST request is used to send data to the server to be processed in some way, like by a CGI script. A POST request is different from a GET request in the following ways:
	//  * There's a block of data sent with the request, in the message body. There are usually extra headers to describe this message body, like Content-Type: and Content-Length:.
	//	* The request URI is not a resource to retrieve; it's usually a program to handle the data you're sending.
	//	* The HTTP response is normally program output, not a static file. 
	// The most common use of POST, by far, is to submit HTML form data to CGI scripts.
	bool POST(const cUrl& url, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam, HttpRequestCompleteCb contentRecvbCb, void* contentCbParam);

	bool SendSoapRequest(const cUrl& url, const cSoapRequest* pSoapRequest, const std::string& soapActionTag, ContentStreamingType streamType, HttpRequestCompleteCb headRecvbCb, void* headCbParam, HttpRequestCompleteCb contentRecvbCb, void* contentCbParam);

	void Process();

	bool IsProcessingRequest() const { return mState != STATE_IDLE; }
	
	bool IsConnecting() const { return (mpSocket && mpSocket->IsOpen() && !mpSocket->ConnectionEstablished()); }
	bool IsConnected() const { return (mpSocket && mpSocket->ConnectionEstablished()); }

	// TODO
	//void ClearLocalHttpCache() const;

private:

	bool Connect(const cUrl& url);

	bool ReadRedundantBytes(u32 numBytes);

	void ProcessState_Connecting();
	void ProcessState_WaitingForReplyHeader();
	void ProcessState_WaitingForContentNormal();
	void ProcessState_WaitingForChunkHeader();
	void ProcessState_WaitingForChunkData();

	void HandleRedirection(u32 replyCode);
	void RequestCannotBeFulFilled(u32 replyCode);

	typedef enum
	{
		STATE_CONNECTING_TO_SERVER =0,
		STATE_WAITING_FOR_REPLY_HEADER,
		STATE_WAITING_FOR_CONTENT_NORMAL,
		STATE_WAITING_FOR_CHUNK_HEADER,
		STATE_WAITING_FOR_CHUNK,
		STATE_IDLE
	}HttpClientState;

	// constants
	enum
	{
		RECV_BUFFER_SIZE = 1024*64,
		DEFAULT_CONNECT_TIMEOUT = 10000
	};

	//STATE_WAITING_FOR_REPLY_HEADER
	//STATE_WAITING_FOR_REPLY_CONTENT_DATA

	// very debug
	void BlockForReplyHeader();
	void BlockForReplyContent();
	
	bool ReceiveReplyHeader();
	s32 ReceiveReplyContent(u32 maxBytes=RECV_BUFFER_SIZE);
	bool ReceiveChunkHeader();

	//void ClearRecvBuffer() { memset(mRecvBuffer, 0, RECV_BUFFER_SIZE); }

	
	typedef enum
	{
		CMD_HEAD = 0,	// Asks for the response identical to the one that would correspond to a GET request, but without the response body. This is useful for retrieving meta-information written in response headers, without having to transport the entire content.
		CMD_GET,		// Requests a representation of the specified resource. By far the most common method used on the Web today. Should not be used for operations that cause side-effects (using it for actions in web applications is a common misuse). See 'safe methods' below.
		CMD_POST,		// Submits data to be processed (e.g. from an HTML form) to the identified resource. The data is included in the body of the request. This may result in the creation of a new resource or the updates of existing resources or both.
		CMD_POST_SOAP,
		CMD_PUT,		// Uploads a representation of the specified resource.
		CMD_DELETE,		// Deletes the specified resource.
		CMD_TRACE,		// Echoes back the received request, so that a client can see what intermediate servers are adding or changing in the request.
		CMD_OPTIONS,	// Returns the HTTP methods that the server supports. This can be used to check the functionality of a web server.
		CMD_CONNECT,	// Converts the request connection to a transparent TCP/IP tunnel, usually to facilitate SSL-encrypted communication (HTTPS) through an unencrypted HTTP
		CMD_INVALID
	}HttpCommand;


	HttpClientState mState;
	
	// not loving this dependency
	cTimer mTimer;
	u32 mRequestTime;
	u32 mConnectTimeout;

	HttpCommand mCommandSent;

	cUrl mUrl;
	IStreamSocket* mpSocket;
	ContentStreamingType mStreamType;

	cByteStream mRecvBuffer;

	cHttpMessageHeader mSendHeader;
	cByteStream mSendContent;

	// TODO : this probably wants to change to a vector of request to support persistent connections and multiple requests
	cHttpMessage mReplyMessage;
	
	cHttpChunkHeader mChunkHeader;
	u32 mCurrentChunkBytesRecvd;

	bool mRedirecting;

	HttpRequestCompleteCb mHeadRcvdCb;
	void* mHeadCbParam;
	HttpRequestCompleteCb mContentRcvdCb;
	void* mContentCbParam;
};



} // namespace net


#endif // _HTTP_H