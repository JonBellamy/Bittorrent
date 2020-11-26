// Jon Bellamy 13-08-2007

#if USE_PCH
#include "stdafx.h"
#endif

#include "NntpClient.h"

#include <assert.h>

#include "Network/WinSock.h"
#include "Network/Dns.h"


namespace net {



cNntpClient::cNntpClient(bool bUseSsl, bool useAuthentication)
: cStreamingTextProtocol(bUseSsl)
//, mUsingAuthentication(useAuthentication)
, mSessionState(IDLE)
{
}// END cNntpClient



cNntpClient::~cNntpClient()
{
}// END ~cNntpClient



bool cNntpClient::Connect()
{
	if(mSocket->IsOpen())
	{
		return false;
	}

	cTextReplyHandler handler;
	handler();


//	mLastCommandSent = CMD_INVALID;

	mSendBuffer.Clear();
	mRecvBuffer.Clear();

	WinSock().Open();

	bool bRet;
	cIpAddr ipAddr; 

	bRet = cDns::IpAddressFromDomainName("news.teleportsv.net", 0, &ipAddr);
	assert(bRet);
	if(!bRet)
	{
		return false;
	}

	u16 port = mUsingSsl ? NNTP_SSL_PORT : NNTP_PORT;
	cSockAddr addr(ipAddr, port);

	printf("Connecting to %s\n", "news.teleportsv.net");

	// blocking !!!
	bRet = mSocket->OpenAndConnect(addr, true);
	assert(bRet);
	if(!bRet)
	{
		return false;
	}

	BlockForNextMessage();
	
	//DisableTTY(true);
	//char sz[] = "LIST\r\n";
	//u32 s = sizeof(sz)-1;	
	//mSocket->Send(sz, sizeof(sz)-1);
	//BlockForNextMessage();
	//GlobalConsoleOutput(gcnew String(reinterpret_cast<char*>(mRecvBuffer.Data())));
	
	

	//SendMessage("LIST", "", true);
	SendMessage("LAST", "", true);
	
	BlockForNextMessage();

	//SendMessage("QUIT", "", true);

	mSessionState = LOGGING_IN;

	return true;
}// END Connect



void cNntpClient::Process()
{
	if(NewMessageFromServer(MULTILINE_EOM))
	{
//		GlobalConsoleOutput(gcnew String(reinterpret_cast<char*>(mRecvBuffer.Data())));
		QuitServer();
	}

	switch(mSessionState)
	{
	case IDLE:
		break;

	case LOGGING_IN:
		break;

	default:
		assert(0);
	}
}// END Process



// TODO
bool cNntpClient::IsErrorMessage()
{
	return false;
}// END IsErrorMessage



void cNntpClient::QuitServer()
{
	printf("cNntpClient::QuitServer\n");
	SendMessage("QUIT", "", true);
	//mSessionState = WAITING_TO_QUIT;
}// END QuitServer



/*
u32 cNntpClient::ReplyCode()
{
	u32 replyCode = 0;

	char* tmp = reinterpret_cast<char*> (&mRecvBuffer[0]);

	u8 code[4];
	code[3] = NULL;
	code[0] = mRecvBuffer[0];
	code[1] = mRecvBuffer[1];
	code[2] = mRecvBuffer[2];

	// note 3 byte return code
	s32 ret = sscanf_s((const char *)code, "%d", &replyCode, 3);

	if(ret == EOF || ret != 1)
	{
		assert(0);
	}

	return replyCode;
}// END ReplyCode



cNntpClient::SmtpReplyCodeType cNntpClient::ReplyCodeType()
{
	s32 replyCode = ReplyCode();

	assert(replyCode >= 100);

	if(replyCode >= SMTP_POSITIVE_PRELIMINARY && replyCode <= s32(SMTP_POSITIVE_PRELIMINARY + 99))
	{
		return SMTP_POSITIVE_PRELIMINARY;
	}

	if(replyCode >= SMTP_POSITIVE_COMPLETION && replyCode <= s32(SMTP_POSITIVE_COMPLETION + 99))
	{
		return SMTP_POSITIVE_COMPLETION;
	}

	if(replyCode >= SMTP_POSITIVE_INTERMEDIATE && replyCode <= s32(SMTP_POSITIVE_INTERMEDIATE + 99))
	{
		return SMTP_POSITIVE_INTERMEDIATE;
	}

	if(replyCode >= SMTP_NEGATIVE_TRANSIENT && replyCode <= s32(SMTP_NEGATIVE_TRANSIENT + 99))
	{
		return SMTP_NEGATIVE_TRANSIENT;
	}

	if(replyCode >= SMTP_NEGATIVE_PERMANENT && replyCode <= s32(SMTP_NEGATIVE_PERMANENT + 99))
	{
		return SMTP_NEGATIVE_PERMANENT;
	}

	assert(0);
	return SMTP_REPLY_UNKNOWN_ERROR;
}// END ReplyCodeType



std::string cNntpClient::ReplyMessage()
{
	s32 index = mRecvBuffer.Find(" ", 1);
	if(index != -1)
	{
		std::string str(reinterpret_cast<const char*> (&mRecvBuffer[index+1]));
		return str;
	}
	return "";
}// END ReplyMessage
*/


} // namespace net

