// Jon Bellamy 02-03-2008



#include "stdafx.h"
#include "MailAccount.h"

#include <assert.h>


#include "Global.h"


namespace net 
{


cMailAccount::cMailAccount()
: mUserName("INVALID")
, mPassword("INVALID")
, mEmailAddress("INVALID")
, mPop3Server("INVALID")
, mSmtpServer("INVALID")
{
}


cMailAccount::cMailAccount(const char* szUserName, const char* szPassword, const char* szEmailAddress, const char* szPop3Server, const char* szSmtpServer)
: mUserName(szUserName)
, mPassword(szPassword)
, mEmailAddress(szEmailAddress)
, mPop3Server(szPop3Server)
, mSmtpServer(szSmtpServer)
{
}


const cMailAccount& cMailAccount::operator= (const cMailAccount& rhs)
{
	mUserName = rhs.mUserName;
	mPassword = rhs.mPassword;
	mEmailAddress = rhs.mEmailAddress;
	mPop3Server = rhs.mPop3Server;
	mSmtpServer = rhs.mSmtpServer;
	return *this;
}// END operator=



cMailAccountManager& MailAccountManager()
{
	static cMailAccountManager accManager;
	return accManager;
}// END MailAccountManager



bool cMailAccountManager::AddAccount(const char* szUserName, const char* szPassword, const char* szEmailAddress, const char* szPop3Server, const char* szSmtpServer)
{
	mMailAccounts.push_back(cMailAccount(szUserName, szPassword, szEmailAddress, szPop3Server, szSmtpServer));
	return true;
}// END AddAccount



} // namespace net