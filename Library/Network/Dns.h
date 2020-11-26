// Jon Bellamy 14-08-2007

#ifndef __DNS_H
#define __DNS_H



#include <WinSock2.h>

#include "Network/NetSettings.h"
#include "WinSock.h"
#include "Network/TcpSocket.h"

namespace net {


class cDns
{

public:
	
	cDns();

	static u8 NumberOfAliasesForDomainName(const char* domainName);

	static bool IpAddressFromDomainName(const char* domainName, u8 aliasNumber, cIpAddr* ipOUT);

private:

};



} // namespace net


#endif 