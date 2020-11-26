// Jon Bellamy 13-08-2007

#include "stdafx.h"

#include "Pop3Client.h"

#include <assert.h>

#include "WinSock.h"
#include "Network/Dns.h"
#include "Network/Mail/Mime.h"


// app specific
#include "file.h"
#include "Global.h"

#include "Network/OpenSslWrapper.h"


namespace net {


cPop3Client::cPop3Client(bool bUseSsl)
: cStreamingTextProtocol(bUseSsl)
, mLastCommandSent(CMD_INVALID)
, mState(IDLE)
, mNumNewMessages(0)
, mCurrentMessageNumber(1)
, mWaitingForMessageDownload(false)
, mWaitingForMessageDeletion(false)
, mCompleteCallback(NULL)
{
}// END cPop3Client



cPop3Client::~cPop3Client()
{
	delete mSocket;
}// END ~cPop3Client



bool cPop3Client::CheckForNewMessages(const cMailAccount& account, Pop3RequestCompleteCb cb)
{
	if(mSocket->IsOpen())
	{
		return false;
	}

	mAccount = account;

	mCompleteCallback = cb;

	mLastCommandSent = CMD_INVALID;
	mCurrentMessageNumber = 1;
	mNumNewMessages = 0;
	mWaitingForMessageDownload = false;
	mWaitingForMessageDeletion = false;

	mSendBuffer.Clear();
	mRecvBuffer.Clear();

	WinSock().Open();

	bool bRet;

	cIpAddr ipAddr; //(195, 188, 53, 61);

	// cDns::NumberOfAliasesForDomainName("pop3.blueyonder.co.uk");
	bRet = cDns::IpAddressFromDomainName(mAccount.Pop3Server().c_str(), 0, &ipAddr);
	assert(bRet);
	if(!bRet)
	{
		return false;
	}

	u16 port = mUsingSsl ? POP3_SSL_PORT : POP3_PORT;
	cSockAddr addr(ipAddr, port);

	printf("Connecting to %s\n", mAccount.Pop3Server().c_str());

	mSocket->OpenAndConnect(addr, false);

	mState = LOGGING_IN;

	return true;
}// END CheckForNewMessages



void cPop3Client::Process()
{
	switch(mState)
	{
	case IDLE:
		break;

	case LOGGING_IN:
		ProcessState_LoggingOn();
		break;

	case MAIL_COUNT:
		ProcessState_MailCount();
		break;

	case DOWNLOADING_MESSAGE:
		ProcessState_DownloadingMessage();
		break;

	case DELETING_MESSAGE:
		ProcessState_DeleteMessage();
		break;

	case WAITING_TO_QUIT:
		ProcessState_WaitingToQuit();
		break;

	default:
		assert(0);
	}
}// END Process



void cPop3Client::ReplyHandler_User(void* pThis, bool isErrorMsg)
{
}// END ReplyHandler_User



// send the username, wait for +OK then send the password and again wait for the +OK
void cPop3Client::ProcessState_LoggingOn()
{
	switch(mLastCommandSent)
	{
	case CMD_INVALID:
		if(NewMessageFromServer(SINGLELINE_EOM.c_str()))
		{
			if(IsErrorMessage())
			{
				QuitServer();
			}
			else
			{
				// USER
				SendMessage(USER, mAccount.UserName().c_str());
				mLastCommandSent = USER;
			}
		}
		break;

	case USER:
		if(NewMessageFromServer(SINGLELINE_EOM.c_str()))
		{
			if(IsErrorMessage())
			{
				QuitServer();
			}
			else
			{
				// PASS
				printf("C: *PASS*\n");
				SendMessage(PASS, mAccount.Password().c_str(), false);
				mLastCommandSent = PASS;
			}
		}
		break;

	case PASS:
		if(NewMessageFromServer(SINGLELINE_EOM.c_str()))
		{
			if(IsErrorMessage())
			{
				QuitServer();
			}
			else
			{
				//printf("User %s logged in\n", mAccount.UserName().c_str());
				mState = MAIL_COUNT;
			}
		}
		break;

	default:
		assert(0);
	}
}// END ProcessState_LoggingOn



// find out how many messages are in the mailbox, if none then quit the server and enter IDLE state
void cPop3Client::ProcessState_MailCount()
{
	if(mLastCommandSent != LIST)
	{
		SendMessage(LIST, "");
		mLastCommandSent = LIST;
	}

	if(NewMessageFromServer(MULTILINE_EOM.c_str()))
	{
		if(IsErrorMessage())
		{
			QuitServer();
		}
		else
		{
			// the LIST command response has arrived, parse the string and find out how many messages are in the mailbox

			// TODO : this is some nasty parsing, write a proper message parser !!!
			char szNumMessage[8];
			int i=4, j=0;
			while(mRecvBuffer.Data()[i] != ' ')
			{
				szNumMessage[j] = mRecvBuffer.Data()[i];
				j++;
				i++;
			}
			szNumMessage[j]=NULL;
			mNumNewMessages = atoi(szNumMessage);

			//printf("%d new messages in mailbox\n", mNumNewMessages);

			if(mNumNewMessages > 0)
			{
				mState = DOWNLOADING_MESSAGE;
			}
			else
			{
				mState = WAITING_TO_QUIT;
			}
		}
	}
}// END ProcessState_MailCount



// download all the new messages
void cPop3Client::ProcessState_DownloadingMessage()
{
	// download the next message
	if(!mWaitingForMessageDownload)
	{
		char szBuf[8];
		sprintf_s(szBuf, 8, "%d", mCurrentMessageNumber);
		SendMessage(RETR, szBuf);
		mLastCommandSent = RETR;
		mWaitingForMessageDownload=true;

		printf("Downloading message %d ...\n", mCurrentMessageNumber);
	}

	DisableTTY(true);
	if(NewMessageFromServer(MULTILINE_EOM.c_str()))
	{
		if(IsErrorMessage())
		{
			QuitServer();
		}
		else
		{
			//std::string s1 = mRecvBuffer.AsString();

			// trim the server reply off
			s32 index = mRecvBuffer.Find(SINGLELINE_EOM.c_str(), SINGLELINE_EOM.size());
			assert(index != -1);
			mRecvBuffer.RemoveBytes(0, index+2);

			//std::string s2 = mRecvBuffer.AsString();

			// save message to file
			CMimeMessage mail;
			int nLoadedSize = mail.Load((const char *)(mRecvBuffer.Data()), mRecvBuffer.Size());

			// Analyze the message header
			const char* pszField;
			pszField = mail.GetFieldValue("Message-ID");
			assert(pszField != NULL);
			//if (pszField != NULL)
			//	printf("X-Priority: %s\r\n", pszField);
			

			char szFn[MAX_PATH];
			strcpy_s(szFn, MAX_PATH, pszField);
			char* pFinalDelimeter = strchr(szFn, '>');
			assert(pFinalDelimeter);
			*pFinalDelimeter = NULL;
			szFn[0] = 'M';

			std::string strFN = "./" + mAccount.UserName() + "/" + szFn + ".txt";
			cFile messageFile(strFN.c_str());
			messageFile.LogMessage("%s", mRecvBuffer.Data());
			messageFile.Close();


			//////////////////////////////////////////////////////////////////////////
			// inc message to download or quit

			printf("message downloaded\n");

			mWaitingForMessageDownload=false;

			mState = DELETING_MESSAGE;
		}
	}
	DisableTTY(false);
}// END ProcessState_DownloadingMessage



void cPop3Client::ProcessState_DeleteMessage()
{
	// download the next message
	if(!mWaitingForMessageDeletion)
	{
		char szBuf[8];
		sprintf_s(szBuf, 8, "%d", mCurrentMessageNumber);
		SendMessage(DELE, szBuf);
		mLastCommandSent = DELE;
		mWaitingForMessageDeletion=true;

		printf("Deleting message %d from server ...\n", mCurrentMessageNumber);
	}

	if(NewMessageFromServer(SINGLELINE_EOM.c_str()))
	{
		if(IsErrorMessage())
		{
			QuitServer();
		}
		else
		{
			mWaitingForMessageDeletion = false;

			mCurrentMessageNumber++;

			// all messages downloaded, quit otherwise downlaod the next one
			if(mCurrentMessageNumber > mNumNewMessages)
			{
				mState = WAITING_TO_QUIT;
			}
			else
			{
				mState = DOWNLOADING_MESSAGE;
			}
		}
	}
}// END ProcessState_DeleteMessage



void cPop3Client::ProcessState_WaitingToQuit()
{
	if(mLastCommandSent != QUIT)
	{
		SendMessage(QUIT, "");
		mLastCommandSent = QUIT;
	}

	if(NewMessageFromServer(SINGLELINE_EOM.c_str()))
	{
		assert(!IsErrorMessage());
	
		mSocket->Close();

		mState = IDLE;

		if(mCompleteCallback)
		{
			mCompleteCallback(mCurrentMessageNumber-1);
		}
	}
}// END ProcessState_WaitingToQuit



void cPop3Client::QuitServer()
{
	printf("Disconnecting from Pop3 server.\n");

	SendMessage(QUIT, "");
	mLastCommandSent = QUIT;
	mState = WAITING_TO_QUIT;
}// END QuitServer



bool cPop3Client::IsErrorMessage()
{
	return mRecvBuffer[0] == '-';
}// END IsErrorMessage




} // namespace net