// Jon Bellamy 13-08-2007

#if USE_PCH
#include "stdafx.h"
#endif

#include "Dns.h"


//#include "Global.h"


namespace net {



cDns::cDns()
{
}


u8 cDns::NumberOfAliasesForDomainName(const char* domainName)
{
	hostent* host;
	host = gethostbyname(domainName);

	if(host==NULL)
	{
		return -1;
	}

	u8 i;
	for (i=0; host->h_addr_list[i] != 0; ++i) 
	{
	}
	return i;
}// END NumberOfAliasesForDomainName



bool cDns::IpAddressFromDomainName(const char* domainName, u8 aliasNumber, cIpAddr* ipOUT)
{
#if DNS_DEBUG_MESSAGES
	Printf("DNS : Resolving %s ...\n", domainName);
#endif

	hostent* host;
	host = gethostbyname(domainName);

	if(host==NULL)
	{
#if DNS_DEBUG_MESSAGES
		Printf("DNS : FAILED to resolve domain.\n");
#endif
		return false;
	}

	//u32 err = WSAGetLastError();

	struct in_addr addr;
	memcpy(&addr, host->h_addr_list[aliasNumber], sizeof(struct in_addr));

	char* sz = inet_ntoa(addr);

	*ipOUT = cIpAddr(addr.S_un.S_un_b.s_b1, addr.S_un.S_un_b.s_b2, addr.S_un.S_un_b.s_b3, addr.S_un.S_un_b.s_b4);

	if(*ipOUT == cIpAddr(127,0,0,1))
	{
		return false;
	}


	char szAddr[32];
	sprintf_s(szAddr, 32, "%d.%d.%d.%d", ipOUT->GetB1(), ipOUT->GetB2(), ipOUT->GetB3(), ipOUT->GetB4());
	
#if DNS_DEBUG_MESSAGES
	Printf("DNS : Resolved IP %s\n", szAddr);
#endif

	return true;
}// END IpAddressFromDomainName



} // namespace net