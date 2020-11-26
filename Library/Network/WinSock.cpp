// Jon Bellamy 14-03-2006
// Win Sock Wrapper


#if USE_PCH
#include "stdafx.h"
#endif

#include "WinSock.h"


namespace net 
{


cWinSock& WinSock()
{
	static cWinSock winSock;
	return winSock;
}// END WinSock


cWinSock::cWinSock() :
mbIsOpen(false)
{
}


bool cWinSock::Open()
{
	if (mbIsOpen)
	{
		return false;
	}

	// Load Winsock 2.0 DLL
	if (WSAStartup(MAKEWORD(2, 2), &mWsaData) != 0) 
	{
		MessageBox(NULL, L"Failed to Start WinSock 2.0", L"ERROR", MB_ICONERROR);
		return false;
	}
	mbIsOpen = true;
	return true;
}// END Open



void cWinSock::Close()
{
	WSACleanup();
}// END Close



} // namespace net