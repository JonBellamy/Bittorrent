// Jon Bellamy 13-08-2007

#ifndef __POP3_H
#define __POP3_H


#include "Network/Core/StreamingTextProtocol.h"


namespace net {


class cPop3Client : public cStreamingTextProtocol
{
public:

	typedef enum
	{
		USER = ('R'<<24)+('E'<<16)+('S'<<8)+('U'),
		PASS = ('S'<<24)+('S'<<16)+('A'<<8)+('P'),
		LIST = ('T'<<24)+('S'<<16)+('I'<<8)+('L'),
		RETR = ('R'<<24)+('T'<<16)+('E'<<8)+('R'),
		DELE = ('E'<<24)+('L'<<16)+('E'<<8)+('D'),
		QUIT = ('T'<<24)+('I'<<16)+('U'<<8)+('Q'),
		NUM_COMMANDS = 5,
		CMD_INVALID = -1
	}Pop3Command;

	typedef void (*Pop3RequestCompleteCb) (u32 numMessageDownloaded);


	cPop3Client(bool bUseSsl);
	~cPop3Client();

	bool CheckForNewMessages(const cMailAccount& account, Pop3RequestCompleteCb cb=NULL);

	void Process();

	typedef enum
	{
		IDLE=0,
		LOGGING_IN,
		MAIL_COUNT,
		DOWNLOADING_MESSAGE,
		DELETING_MESSAGE,
		WAITING_TO_QUIT,
		NUM_STATES,
	}Pop3ClientState;

	Pop3ClientState State() const { return mState; }

private:	

	bool IsErrorMessage();

	void QuitServer();

	static void ReplyHandler_User(void* pThis, bool isErrorMsg);

	void ProcessState_LoggingOn();
	void ProcessState_MailCount();
	void ProcessState_DownloadingMessage();
	void ProcessState_DeleteMessage();
	void ProcessState_WaitingToQuit();

	Pop3Command mLastCommandSent;

	Pop3ClientState mState;

	u32 mNumNewMessages;
	u32 mCurrentMessageNumber;
	bool mWaitingForMessageDownload;
	bool mWaitingForMessageDeletion;

	Pop3RequestCompleteCb mCompleteCallback;
};



} // namespace net


#endif 