// Jon Bellamy 13-08-2007

#include "stdafx.h"

#include "SMTPClient.h"

#include <assert.h>

#include "Network/WinSock.h"
#include "Network/Dns.h"
#include "Network/Mail/Mime.h"
#include "Network/Base64.h"

// app specific
#include "file.h"
#include "Global.h"


namespace net {



cSmtpClient::cSmtpClient(bool bUseSsl, bool useAuthentication)
: cStreamingTextProtocol(bUseSsl)
, mUsingAuthentication(useAuthentication)
, mLastCommandSent(CMD_INVALID)
, mSessionState(IDLE)
, mLoginState(LOGIN_IDLE)
{
}// END cSMTPClient



cSmtpClient::~cSmtpClient()
{
}// END ~cSMTPClient



bool cSmtpClient::SendMail(const cMailAccount& account, const CMimeMessage& msg)
{
	if(mSocket->IsOpen())
	{
		return false;
	}

	mAccount = account;

	// WARNING : i wrote the operator= in the mime class, if things start going south check this first
	mSendMessage = msg;

	mLastCommandSent = CMD_INVALID;

	mSendBuffer.Clear();
	mRecvBuffer.Clear();

	WinSock().Open();

	bool bRet;
	cIpAddr ipAddr; 

	bRet = cDns::IpAddressFromDomainName(mAccount.SmtpServer().c_str(), 0, &ipAddr);
	assert(bRet);
	if(!bRet)
	{
		return false;
	}

	u16 port = mUsingSsl ? SMTP_SSL_PORT : SMTP_PORT;
	cSockAddr addr(ipAddr, port);

	printf("Connecting to %s\n", mAccount.SmtpServer().c_str());
	
	bRet = mSocket->OpenAndConnect(addr, false);
	assert(bRet);
	if(!bRet)
	{
		return false;
	}

	//BlockForNextMessage();

	mSessionState = LOGGING_IN;
	mLoginState = LOGIN_IDLE;

	return true;
}// END SendMail



void cSmtpClient::Process()
{
	switch(mSessionState)
	{
	case IDLE:
		break;

	case LOGGING_IN:
		ProcessState_LoggingIn();
		break;

	case SENDING_MAIL:
		ProcessState_SendingMail();
		break;

	case WAITING_TO_QUIT:
		ProcessState_WaitingToQuit();
		break;


	case MAIL_SENT:
		ProcessState_MailSent();
		break;

	default:
		assert(0);
	}
}// END Process



// first we send EHLO, if the server cannot handle it we fallback on HELO. After that we are done unless we 
// want authentication in which case we use the SASL LOGIN mechanism, send username then password both Base64 encoded.
void cSmtpClient::ProcessState_LoggingIn()
{
	assert(mSessionState == LOGGING_IN);

	if(!NewMessageFromServer(SINGLELINE_EOM.c_str()))
	{
		return;
	}

	switch(mLoginState)
	{
	case LOGIN_IDLE:
		// TODO : what should this be?
		SendMessage(EHLO, "www.themeerkat.net");
		mLastCommandSent = EHLO;
		mLoginState = EHLO_SENT;
		break;

	case EHLO_SENT:
		if(IsErrorMessage())
		{
			// EHLO isn't working, fallback on HELO (cannot do authentication now)				
			SendMessage(HELO, "www.themeerkat.net");
			mLastCommandSent = HELO;
			mLoginState = HELO_SENT;
		}
		else
		{
			// do we need to authenticate?
			if(mUsingAuthentication)
			{				
				SendMessage(AUTH, "LOGIN");
				mLastCommandSent = AUTH;
				mLoginState = AUTH_METHOD_SENT;
			}
			else
			{
				OnEnterStateSendingMail();
			}
		}
		break;


	case HELO_SENT:
		assert(mUsingAuthentication == false);
		OnEnterStateSendingMail();
		break;


	case AUTH_METHOD_SENT:
		{
		// decode and output the servers reply
		std::string replyStr = DecodeBase64(ReplyMessage().c_str());
		printf("Base64 reply : %s\n", replyStr.c_str());

		// encode and send our response, they didn't stick with the CMD PARAM syntax.
		mRecvBuffer.Clear();
		std::string b64Str = net::EncodeBase64(reinterpret_cast<const u8*>(mAccount.UserName().c_str()), mAccount.UserName().size());
		mSocket->Send(b64Str.c_str(), b64Str.size());	
		mSocket->Send(SINGLELINE_EOM.c_str(), SINGLELINE_EOM.size());
		printf("C: %s\n", mAccount.UserName().c_str());
		mLoginState = USERNAME_SENT;
		}
		break;


	case USERNAME_SENT:
		{
			if(IsErrorMessage())
			{
				mLoginState = LOGIN_COMPLETE_FAIL;
				QuitServer();
				return;
			}

			// decode and output the servers reply
			std::string replyStr = DecodeBase64(ReplyMessage().c_str());
			printf("Base64 reply : %s\n",replyStr.c_str());

			// encode and send our response
			mRecvBuffer.Clear();
			std::string b64Str = net::EncodeBase64(reinterpret_cast<const u8*>(mAccount.Password().c_str()), mAccount.Password().size());
			mSocket->Send(b64Str.c_str(), b64Str.size());			// trim the NULL
			mSocket->Send(SINGLELINE_EOM.c_str(), SINGLELINE_EOM.size());
			printf("C: *pass*\n");
			mLoginState = PASSWORD_SENT;
		}
		break;


	case PASSWORD_SENT:
		{
			if(IsErrorMessage())
			{
				mLoginState = LOGIN_COMPLETE_FAIL;
				QuitServer();
				return;
			}

			OnEnterStateSendingMail();
		}
		break;
	}
}// END ProcessState_LoggingIn



void cSmtpClient::OnEnterStateSendingMail()
{
	mLoginState = LOGIN_COMPLETE_OK;
	std::string str = "From:<" + mAccount.EmailAddress() + ">";
	SendMessage(MAIL, str.c_str());
	mLastCommandSent = MAIL;
	mSessionState = SENDING_MAIL;
}// END OnEnterStateSendingMail



// send the username, wait for +OK then send the password and again wait for the +OK
void cSmtpClient::ProcessState_SendingMail()
{
	if(!NewMessageFromServer(SINGLELINE_EOM.c_str()))
	{
		return;
	}
	else
	{
		if(IsErrorMessage())
		{
			QuitServer();
			return;
		}
	}

	switch(mLastCommandSent)
	{
	case MAIL:
		{
		std::string mailAddr(mSendMessage.GetTo());
		std::string str = "To:<" + mailAddr + ">";

		SendMessage(RCPT, str.c_str());
		mLastCommandSent = RCPT;
		}
		break;

	case RCPT:
		SendMessage(DATA, NULL);
		mLastCommandSent = DATA;
		break;

	case DATA:
		SendMailData();
		break;

	default:
		assert(0);
	}
}// END ProcessState_SendingMail



// waits for the confirmation that the mail has gone then quits
void cSmtpClient::ProcessState_MailSent()
{
	if(!NewMessageFromServer(SINGLELINE_EOM.c_str()))
	{
		return;
	}
	QuitServer();
}// END ProcessState_MailSent



void cSmtpClient::ProcessState_WaitingToQuit()
{
	if(mLastCommandSent != QUIT)
	{
		SendMessage(QUIT, "");
		mLastCommandSent = QUIT;
	}

	if(NewMessageFromServer(SINGLELINE_EOM.c_str()))
	{
		//assert(!IsErrorMessage());
	
		printf("Disconnected from SMTP server.\n");

		mSocket->Close();

		mSessionState = IDLE;
	}
}// END ProcessState_WaitingToQuit



void cSmtpClient::QuitServer()
{
	printf("cSMTPClient::QuitServer\n");
	SendMessage(QUIT, "");
	mLastCommandSent = QUIT;
	mSessionState = WAITING_TO_QUIT;
}// END QuitServer



void cSmtpClient::SendMailData()
{
	mSendBuffer.Clear();	

	u32 size = mSendMessage.GetLength();
	
	// TODO : this should be a bytesstream
	char* sz = new char[size];
	
	mSendMessage.Store(sz, size);

	mSocket->Send(sz, size);

	delete[] sz;

	char szTerminator[] = "\r\n.\r\n";
	mSocket->Send(szTerminator, sizeof(szTerminator)-1);

	mLastCommandSent = THE_DATA;

	mSessionState = MAIL_SENT;
}// END SendMailData



bool cSmtpClient::IsErrorMessage()
{
	return (ReplyCodeType() == SMTP_NEGATIVE_TRANSIENT || 
		    ReplyCodeType() == SMTP_NEGATIVE_PERMANENT);
}// END IsErrorMessage



u32 cSmtpClient::ReplyCode()
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



cSmtpClient::SmtpReplyCodeType cSmtpClient::ReplyCodeType()
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



std::string cSmtpClient::ReplyMessage()
{
	s32 index = mRecvBuffer.Find(" ", 1);
	if(index != -1)
	{
		std::string str(reinterpret_cast<const char*> (&mRecvBuffer[index+1]));
		return str;
	}
	return "";
}// END ReplyMessage










} // namespace net