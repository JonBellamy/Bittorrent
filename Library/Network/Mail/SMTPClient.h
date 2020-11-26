// Jon Bellamy 23-12-2007

#ifndef __SMTP_H
#define __SMTP_H



#include <WinSock2.h>

#include "Network/Core/StreamingTextProtocol.h"
#include "Network/Mail/Mime.h"


namespace net {


class cSmtpClient : public cStreamingTextProtocol
{
public:

	// TODO : both of these should maybe be passed to SendMail() instead
	cSmtpClient(bool bUseSsl, bool useAuthentication);
	~cSmtpClient();

	bool SendMail(const cMailAccount& account, const CMimeMessage& msg);

	void Process();

	void QuitServer();


	typedef enum
	{
		IDLE=0,
		LOGGING_IN,
		SENDING_MAIL,
		MAIL_SENT,
		WAITING_TO_QUIT,
		NUM_STATES,
	}SMTPClientState;

	typedef enum
	{
		LOGIN_IDLE =0,
		EHLO_SENT,
		HELO_SENT,
		AUTH_METHOD_SENT,
		USERNAME_SENT,
		PASSWORD_SENT,
		LOGIN_COMPLETE_OK,
		LOGIN_COMPLETE_FAIL,
		NUM_LOGIN_STATES,
	}SMTPLoginState;

	typedef enum
	{
		SMTP_POSITIVE_PRELIMINARY	= 100,			// 1XX
		SMTP_POSITIVE_COMPLETION	= 200,			// 2XX
		SMTP_POSITIVE_INTERMEDIATE	= 300,			// 3XX
		SMTP_NEGATIVE_TRANSIENT		= 400,			// 4XX
		SMTP_NEGATIVE_PERMANENT		= 500,			// 5XX

		SMTP_REPLY_UNKNOWN_ERROR = 999
	}SmtpReplyCodeType;


	typedef enum
	{
		HELO = ('O'<<24)+('L'<<16)+('E'<<8)+('H'),
		EHLO = ('O'<<24)+('L'<<16)+('H'<<8)+('E'),
		MAIL = ('L'<<24)+('I'<<16)+('A'<<8)+('M'),
		RCPT = ('T'<<24)+('P'<<16)+('C'<<8)+('R'),
		DATA = ('A'<<24)+('T'<<16)+('A'<<8)+('D'),
		RSET = ('T'<<24)+('E'<<16)+('S'<<8)+('R'),
		HELP = ('P'<<24)+('L'<<16)+('E'<<8)+('H'),
		QUIT = ('T'<<24)+('I'<<16)+('U'<<8)+('Q'),

		// SMTP extensions
		AUTH = ('H'<<24)+('T'<<16)+('U'<<8)+('A'),

		NUM_COMMANDS = 9,
		THE_DATA = 100,				// the actual data as opposed to the data command
		CMD_INVALID = -1
	}SMTPCommand;


	SMTPClientState State() const { return mSessionState; }

private:	

	void OnEnterStateSendingMail();

	void SendMailData();	
	bool IsErrorMessage();

	u32 ReplyCode();
	SmtpReplyCodeType ReplyCodeType();
	std::string ReplyMessage();


	// TODO : timeouts from all states

	void ProcessState_LoggingIn();
	void ProcessState_SendingMail();
	void ProcessState_WaitingToQuit();
	void ProcessState_MailSent();

	bool mUsingAuthentication;

	SMTPCommand mLastCommandSent;

	// master state
	SMTPClientState mSessionState;

	// sub state's
	SMTPLoginState mLoginState; 

	CMimeMessage mSendMessage;
};



} // namespace net


#endif 