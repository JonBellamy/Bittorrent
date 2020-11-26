// Jon Bellamy 14-03-2006
// Win Sock Wrapper


#ifndef __WINSOCK_H
#define __WINSOCK_H


#include <WinSock2.h>

#include "Network/NetSettings.h"

namespace net 
{


class cWinSock
{

public:
	
	cWinSock();

	bool Open();
	void Close();

private:

	WSADATA mWsaData;					// Structure for WinSock setup communication 

	bool mbIsOpen;
};


// Singleton
cWinSock& WinSock();


} // namespace net


#endif 