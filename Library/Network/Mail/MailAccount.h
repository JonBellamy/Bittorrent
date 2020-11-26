// Jon Bellamy 02-03-2008



#ifndef _MAIL_ACCOUNT_H
#define _MAIL_ACCOUNT_H


#include <string>
#include <vector>

namespace net 
{



class cMailAccount
{
public:	
	cMailAccount();
	cMailAccount(const char* szUserName, const char* szPassword, const char* szEmailAddress, const char* szPop3Server, const char* szSmtpServer);

	const cMailAccount& operator= (const cMailAccount& rhs);

	void UserName(const std::string& str) { mUserName = str; }
	void Password(const std::string& str) { mPassword = str; }
	void EmailAddress(const std::string& str) { mEmailAddress = str; }
	void Pop3Server(const std::string& str) { mPop3Server = str; }
	void SmtpServer(const std::string& str) { mSmtpServer = str; }

	const std::string& UserName() const { return mUserName; }
	const std::string& Password() const { return mPassword; }
	const std::string& EmailAddress() const { return mEmailAddress; }
	const std::string& Pop3Server() const { return mPop3Server; }
	const std::string& SmtpServer() const { return mSmtpServer; }

private:

	std::string mUserName;
	std::string mPassword;
	std::string mEmailAddress;
	std::string mPop3Server;
	std::string mSmtpServer;
};



class cMailAccountManager
{
public:	
	cMailAccountManager() {}

	bool AddAccount(const char* szUserName, const char* szPassword, const char* szEmailAddress, const char* szPop3Server, const char* szSmtpServer);

	const cMailAccount& Account(u32 index) { return mMailAccounts[index]; }
	u32 NumberOfAccounts() const { return mMailAccounts.size(); }

private:

	std::vector<cMailAccount> mMailAccounts;
};


extern cMailAccountManager& MailAccountManager();


} // namespace net


#endif // _MAIL_ACCOUNT_H