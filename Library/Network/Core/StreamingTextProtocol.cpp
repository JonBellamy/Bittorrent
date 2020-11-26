// Jon Bellamy 27-01-2008

#if USE_PCH
#include "stdafx.h"
#endif

#include "StreamingTextProtocol.h"



namespace net 
{


cStreamingTextProtocol::cStreamingTextProtocol(bool bUseSsl)
: mUsingSsl(bUseSsl)
, mRecvBuffer(RECV_BUFFER_SIZE)
, mSendBuffer(SEND_BUFFER_SIZE)
, SINGLELINE_EOM("\r\n")
, MULTILINE_EOM("\r\n.\r\n")
, mDisableTTY(false)
{
	if(bUseSsl)
	{
		mSocket = new cSslSocket;	
//		static_cast<cSslSocket*>(mSocket)->UseVerification("cert.pem", NULL);
	}
	else
	{
		mSocket = new cTcpSocket;
	}
}// END cStreamingTextProtocol



cStreamingTextProtocol::~cStreamingTextProtocol()
{
	delete mSocket;
	mSocket=NULL;

	mRecvBuffer.Clear();
	mSendBuffer.Clear();
}// END ~cStreamingTextProtocol



void cStreamingTextProtocol::Process()
{
	const char* EOM = mLastMessageSent.ExpectMultipluneResponse() ? MULTILINE_EOM.c_str() : SINGLELINE_EOM.c_str(); 
	if(mReplyHandler && NewMessageFromServer(EOM))
	{
		mReplyHandler(IsErrorMessage());
	}
}// END Process



void cStreamingTextProtocol::SendMessage(const cStreamingTextProtocolMessage& msg, MessageReplyHandler replyHandler, bool outputReply)
{
	mLastMessageSent = msg;
	mReplyHandler = replyHandler;

	mSendBuffer.Clear();

	mSendBuffer.StreamBytes(reinterpret_cast<const u8*>(msg.Command().c_str()), msg.Command().size());

	if(msg.Param().size())
	{
		const u8 space(' ');
		mSendBuffer.StreamBytes(&space, 1);

		mSendBuffer.StreamBytes(reinterpret_cast<const u8*>(msg.Param().c_str()), msg.Param().size());
	}

	mSendBuffer.StreamBytes(reinterpret_cast<const u8*>(SINGLELINE_EOM.c_str()), SINGLELINE_EOM.size());

	mRecvBuffer.Clear();

	if(outputReply)
	{
		printf("C: %s", reinterpret_cast<char*>(mSendBuffer.Data()));
	}

	// VERY DEBUG
	//std::string debugStr = mSendBuffer.AsString();

	mSocket->Send(mSendBuffer.Data(), mSendBuffer.Size());

	mDisableTTY = !outputReply;
}// END SendMessage



void cStreamingTextProtocol::SendMessage(const std::string& cmd, const std::string& pParam, bool outputReply)
{
	mSendBuffer.Clear();

	mSendBuffer.StreamBytes(reinterpret_cast<const u8*>(cmd.c_str()), cmd.size());

	if(pParam.size())
	{
		const u8 space(' ');
		mSendBuffer.StreamBytes(&space, 1);

		mSendBuffer.StreamBytes(reinterpret_cast<const u8*>(pParam.c_str()), pParam.size());
	}

	mSendBuffer.StreamBytes(reinterpret_cast<const u8*>(SINGLELINE_EOM.c_str()), SINGLELINE_EOM.size());

	mRecvBuffer.Clear();

	if(outputReply)
	{
		printf("C: %s", reinterpret_cast<char*>(mSendBuffer.Data()));
	}

	// VERY DEBUG
	//std::string debugStr = mSendBuffer.AsString();

	mSocket->Send(mSendBuffer.Data(), mSendBuffer.Size());
	
	mDisableTTY = !outputReply;
}// END SendMessage



void cStreamingTextProtocol::SendMessage(u32 cmd, const std::string& pParam, bool outputReply)
{
	SendMessage(U32CommandToString(cmd), pParam, outputReply);
}// END SendMessage



std::string cStreamingTextProtocol::U32CommandToString(u32 cmd)
{
	char szCmd[8];
	memcpy(szCmd, &cmd, sizeof(cmd));
	szCmd[4]=NULL;
	return std::string(szCmd);
}// END U32CommandToString




void cStreamingTextProtocol::OutputLastRecvdMessage()
{
	if(mDisableTTY)
	{
		return; 
	}
	// TODO : old data is still in the buffer and is being output
	printf("S: %s", reinterpret_cast<char*>(&mRecvBuffer[0]));
}// END OutputLastRecvdMessage



// returns true if we have completely received a new message from the server
bool cStreamingTextProtocol::NewMessageFromServer(const std::string& endOfMessageId)
{
	u8 buf[RECV_BUFFER_SIZE];
	int bytesRecvd = mSocket->Recv(buf, RECV_BUFFER_SIZE, false);

	if(bytesRecvd > 0)
	{
		//printf("bytesRecvd == %d\n", bytesRecvd);

		mRecvBuffer.StreamBytes(buf, bytesRecvd);

		if(mRecvBuffer.Find(endOfMessageId.c_str(), endOfMessageId.size()) != -1)
		{
			OutputLastRecvdMessage();
			return true;
		}
	}
	return false;
}// END NewMessageFromServer



//////////////////////////////////////////////////////////////////////////
// DEBUG

void cStreamingTextProtocol::BlockForNextMessage()
{
	while(!NewMessageFromServer(SINGLELINE_EOM));
}// END BlockForNextMessage



//////////////////////////////////////////////////////////////////////////
// Protocol Message



cStreamingTextProtocolMessage::cStreamingTextProtocolMessage()
: mCmd("")
, mParam("")
, mbExpectingMultilineReply(false)
{
}// END cStreamingTextProtocolMessage


cStreamingTextProtocolMessage::cStreamingTextProtocolMessage(const std::string& cmd, const std::string& param, bool bExpectingMultilineReply)
: mCmd(cmd)
, mParam(param)
, mbExpectingMultilineReply(bExpectingMultilineReply)
{
}// END cStreamingTextProtocolMessage



cStreamingTextProtocolMessage::cStreamingTextProtocolMessage(u32 cmd, const std::string& param, bool bExpectingMultilineReply)
: mParam(param)
, mbExpectingMultilineReply(bExpectingMultilineReply)
{
	mCmd = U32CommandToString(cmd);
}// END cStreamingTextProtocolMessage



cStreamingTextProtocolMessage::cStreamingTextProtocolMessage(const cStreamingTextProtocolMessage& rhs)
{
	*this = rhs;
}// END cStreamingTextProtocolMessage



const cStreamingTextProtocolMessage& cStreamingTextProtocolMessage::operator= (const cStreamingTextProtocolMessage& rhs)
{
	mCmd = rhs.mCmd;
	mParam = rhs.mParam;
	mbExpectingMultilineReply = rhs.mbExpectingMultilineReply;
	return *this;
}// END operator=



bool cStreamingTextProtocolMessage::operator== (const cStreamingTextProtocolMessage& rhs)
{
	return (mCmd == rhs.mCmd && 
		    mParam == rhs.mParam && 
			mbExpectingMultilineReply == rhs.mbExpectingMultilineReply);
}// END operator==



std::string cStreamingTextProtocolMessage::U32CommandToString(u32 cmd)
{
	char szCmd[8];
	memcpy(szCmd, &cmd, sizeof(cmd));
	szCmd[4]=NULL;
	return std::string(szCmd);
}// END U32CommandToString



void cTextReplyHandler::operator() ()
{
	int x = 10;
	printf("swoo %d\n", x);
}


} // namespace net