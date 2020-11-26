// Jon Bellamy 19-08-2007
// Deals with storing mails on the local hard disk


#ifndef __MAILBOX_H
#define __MAILBOX_H

#include <map>
#include <vector>
#include "Network/Mail/EMailMessage.h"


namespace net {


class cLocalMailBox
{
public:

	cLocalMailBox(const char* userName);
	~cLocalMailBox();

	void PrintAllMessages();

	void Refresh();

	void ClearInbox() { mMsgs.clear(); }

	cEmailMessage* Message(u32 index);

	u32 MessageCount() const { return mMsgs.size(); }


	u32 OutboxMessageCount() const { return mOutbox.size(); }
	const CMimeMessage& OutboxMessage(u32 index) const { return mOutbox[index]; }
	void AddOutboxMessage(const CMimeMessage& msg);


private:
	cLocalMailBox(const cLocalMailBox&);
	const cLocalMailBox& operator=(const cLocalMailBox&);

public:

	static bool MailBoxExists(const char* userName);

	u32 NumberOfMessages() const;

private:

	bool AddMessage(const cEmailMessage& msg);

	void CreateMailBox(const char* userName);

	static void FoundMessageCB(char* pFileName, void* pParam);


private:	

	// all mails, sorted by recv date (as a string)
	typedef pair <std::string, cEmailMessage> MsgPair;
	typedef map<std::string, cEmailMessage>::iterator MsgIterator;
	typedef const map<std::string, cEmailMessage>::iterator ConstMsgIterator;

	std::string mUserName;

	std::map<std::string, cEmailMessage> mMsgs;

	std::vector<CMimeMessage> mOutbox;
};



} // namespace net


#endif 