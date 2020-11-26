// Jon Bellamy 27-01-2008

#ifndef _PROTCLIENT_H
#define _PROTCLIENT_H


#include "Network/TcpSocket.h"



namespace net 
{

/*
class cBasicProtocolClient
{
public:

	cBasicProtocolClient();
	~cBasicProtocolClient();

	bool CheckForNewMessages(const char* szPop3ServerName, const char* userName, const char* password);

	void Process();

	// DEBUG
	void BlockForNextMessage();

	void SendMessage(Pop3Command cmd, const char* pParam=NULL);

	bool IsErrorMessage(const u8* serverMsg);

	void QuitServer();

	// constants
	enum
	{
		RECV_BUFFER_SIZE = 1024*64,
		SEND_BUFFER_SIZE = 1024,
		COMMAND_CODE_SIZE = 4			// 4 byte codes (EG USER)
	};


	typedef enum
	{
		IDLE=0,
		LOGGING_IN,
		MAIL_COUNT,
		DOWNLOADING_MESSAGES,
		WAITING_TO_QUIT,
		NUM_STATES,
	}BasicProtocolClientState;

	BasicProtocolClientState State() const { return mState; }

private:	

	void ClearSendBuffer() { memset(mSendBuffer, 0, RECV_BUFFER_SIZE); }
	void ClearRecvBuffer() { memset(mRecvBuffer, 0, RECV_BUFFER_SIZE); }


	// returns true if we have completely recevied a new message from the server
	bool NewMessageFromServer();


	// TODO : timeouts from all states


	void ProcessState_LoggingOn();
	void ProcessState_MailCount();
	void ProcessState_DownloadingMessages();
	void ProcessState_WaitingToQuit();

	u8 mSendBuffer[SEND_BUFFER_SIZE];
	u8 mRecvBuffer[RECV_BUFFER_SIZE];

	u32 mLastRecvMessageSize;

	net::cTcpSocket mSocket;

	Pop3Command mLastCommandSent;

	BasicProtocolClientState mState;

	u32 mNumNewMessages;
	u32 mCurrentDownloadMessage;
	bool mWaitingForMessageDownload;
};

*/

} // namespace net


#endif // _PROTCLIENT_H