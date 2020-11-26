// Jon Bellamy 27-01-2008


#ifndef _BASIC_TEXT_PROTO_H
#define _BASIC_TEXT_PROTO_H

#include "Network/Ports.h"
#include "Network/TcpSocket.h"
#include "Network/SslSocket.h"
//#include "Network/Mail/MailAccount.h"
#include "Network/ByteStream.h"


namespace net 
{


class cTextReplyHandler
{
public:
	cTextReplyHandler() {}

	void operator() ();

	typedef void (*MessageReplyHandler) (bool isErrorMsg);

private:

	
	MessageReplyHandler m;
};


class cStreamingTextProtocolMessage
{
public:
	cStreamingTextProtocolMessage();
	cStreamingTextProtocolMessage(const std::string& cmd, const std::string& param, bool bExpectingMultilineReply);
	cStreamingTextProtocolMessage(u32 cmd, const std::string& param, bool bExpectingMultilineReply);
	cStreamingTextProtocolMessage(const cStreamingTextProtocolMessage& rhs);

	virtual ~cStreamingTextProtocolMessage() {}

	const cStreamingTextProtocolMessage& operator= (const cStreamingTextProtocolMessage& rhs);
	bool operator== (const cStreamingTextProtocolMessage& rhs);

	bool ExpectMultipluneResponse() const { return mbExpectingMultilineReply; }
	const std::string& Command() const { return mCmd; }
	const std::string& Param() const { return mParam; }

private:

	std::string U32CommandToString(u32 cmd);

	std::string mCmd;
	std::string mParam;
	
	bool mbExpectingMultilineReply;
};



class cStreamingTextProtocol
{
public:

	cStreamingTextProtocol(bool bUseSsl);
	virtual ~cStreamingTextProtocol();

	typedef void (*MessageReplyHandler) (bool isErrorMsg);

	void DisableTTY(bool b) { mDisableTTY = b; }
	bool TTYDisabled() const { return mDisableTTY; }

	virtual bool IsErrorMessage() =0;


protected:
	
	// constants
	enum
	{
		RECV_BUFFER_SIZE = 1024*64,
		SEND_BUFFER_SIZE = 1024,
		COMMAND_CODE_SIZE = 4			// 4 byte codes (EG USER)
	};

	virtual void Process();

	virtual void SendMessage(const std::string& cmd, const std::string& pParam, bool outputReply=true);

	virtual void SendMessage(u32 cmd, const std::string& pParam, bool outputReply=true);
	
	void SendMessage(const cStreamingTextProtocolMessage& msg, MessageReplyHandler replyHandler, bool outputReply=true);

	// returns true if we have completely received a new message from the server
	bool NewMessageFromServer(const std::string& endOfMessageId);

	// DEBUG
	void BlockForNextMessage();

	void OutputLastRecvdMessage();
	
	net::IStreamSocket* mSocket;

	bool mUsingSsl;

	cByteStream mRecvBuffer;
	cByteStream mSendBuffer;
	
	
	// TODO : MOVE THIS !!!!
//	cMailAccount mAccount;

	const std::string SINGLELINE_EOM;
	const std::string MULTILINE_EOM;

private:	

	std::string U32CommandToString(u32 cmd);

	bool mDisableTTY;

	cStreamingTextProtocolMessage mLastMessageSent;
	MessageReplyHandler mReplyHandler;
};



} // namespace net


#endif // _PROTCLIENT_H