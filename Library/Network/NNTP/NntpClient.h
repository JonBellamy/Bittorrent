// Jon Bellamy 23-04-2008

#ifndef __NNTP_H
#define __NNTP_H


#include <string>

#include "Network/Core/StreamingTextProtocol.h"



namespace net {


class cNntpClient : public cStreamingTextProtocol
{
public:
	cNntpClient(bool bUseSsl, bool useAuthentication);
	~cNntpClient();

	bool IsErrorMessage();

	bool Connect();
	
	void Process();

	void QuitServer();


	typedef enum
	{
		IDLE=0,
		LOGGING_IN,
		NUM_STATES,
	}NntpSessionState;

	
/*
	typedef enum
	{
		SMTP_POSITIVE_PRELIMINARY	= 100,			// 1XX
		SMTP_POSITIVE_COMPLETION	= 200,			// 2XX
		SMTP_POSITIVE_INTERMEDIATE	= 300,			// 3XX
		SMTP_NEGATIVE_TRANSIENT		= 400,			// 4XX
		SMTP_NEGATIVE_PERMANENT		= 500,			// 5XX

		SMTP_REPLY_UNKNOWN_ERROR = 999
	}SmtpReplyCodeType;
*/


//	NntpClientState State() const { return mSessionState; }

private:	


	// master state
	NntpSessionState mSessionState;

	const std::string LIST;
};



} // namespace net


#endif 

