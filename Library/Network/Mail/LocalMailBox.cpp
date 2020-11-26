// Jon Bellamy 13-08-2007
// Deals with storing mails on the local hard disk


#include "stdafx.h"
#include "LocalMailBox.h"

#include <assert.h>

#include "MiscLib.h"
#include "Global.h"


namespace net {



cLocalMailBox::cLocalMailBox(const char* userName)
: mUserName(userName)
{		
	Refresh();
}// END cLocalMailBox



void cLocalMailBox::Refresh()
{
	assert(MailBoxExists(mUserName.c_str()));
	ClearInbox();
	std::string path = "./" + mUserName;
	MiscLib::ScanForFileTypes(path.c_str(), "txt", &cLocalMailBox::FoundMessageCB, this);
}// END Refresh



cLocalMailBox::~cLocalMailBox()
{
}// END ~cLocalMailBox



bool cLocalMailBox::MailBoxExists(const char* userName)
{
	return MiscLib::IsDirectory(userName);
}// END MailBoxExists



void cLocalMailBox::AddOutboxMessage(const CMimeMessage& msg)
{
	mOutbox.push_back(msg);
}// END AddOutboxMessage


bool cLocalMailBox::AddMessage(const cEmailMessage& msg)
{
	std::string str(msg.Date());
	MsgPair msgPair(str, msg);
	mMsgs.insert(msgPair);
	return msg.ValidMessage();
}// END AddMessage



void cLocalMailBox::FoundMessageCB(char* pFileName, void* pParam)
{
	cLocalMailBox* pThis = static_cast<cLocalMailBox*> (pParam);

	net::cEmailMessage mail(pFileName);

	const char* sz1 = mail.From();
	const char* sz2 = mail.Date();
	const char* sz3 = mail.Subject();

	bool bRet = pThis->AddMessage(mail);
	assert(bRet);
}// END FoundMessageCB



cEmailMessage* cLocalMailBox::Message(u32 index)
{
	u32 i=0;

	for (MsgIterator& pIter = mMsgs.begin(); pIter != mMsgs.end(); pIter++, i++)
	{
		if(i == index)
		{
			cEmailMessage& msg = pIter->second;
			return &msg;
		}
	}
	return NULL;
}// END Message



void cLocalMailBox::PrintAllMessages() 
{
	//MsgPair msgPair;

	u32 i=0;

	for ( MsgIterator& pIter = mMsgs.begin(); pIter != mMsgs.end(); pIter++, i++)
	{
		cEmailMessage& msg = pIter->second;
		GlobalConsoleOutput(String::Format(L"Msg {0} - From {1} Received : {2}\n", i, gcnew String(msg.From()), gcnew String(msg.Date())));
		GlobalConsoleOutput(String::Format(L"Subject : {0}\n", gcnew String(msg.Subject())));
		GlobalConsoleOutput(String::Format(L"Body : {0}\n", gcnew String(msg.Body(false).c_str())));
	}
}// END PrintAllMessages


} // namespace net