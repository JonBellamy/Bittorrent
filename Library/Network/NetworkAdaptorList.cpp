#include "NetworkAdaptorList.h"

#include "Network/WinSock.h"

namespace net 
{


cNetworkAdaptorList& NetworkAdaptorList()
{
	static cNetworkAdaptorList nal;
	return nal;
}// END NetworkAdaptorList



cNetworkAdaptorList::cNetworkAdaptorList() 
{ 
	WinSock().Open();
	Refresh(); 
}// END cNetworkAdaptorList
	


bool cNetworkAdaptorList::Refresh()
{
	mLocalIpList.clear();

	struct 
	{
		INT iAddressCount;
		SOCKET_ADDRESS Address[32];		// <<-- do we need this or can we use [1] like in the docs?
	}slist;

	SOCKET sock;
	DWORD dwBytesRet;
	int ret;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sock == SOCKET_ERROR) 
	{
		Printf("socket(AF_INET, SOCK_STREAM, IPPROTO_IP) failed - error code: %u\n", WSAGetLastError());
		return false;
	}
	ret = WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY, NULL, 0, &slist, sizeof(slist), &dwBytesRet, NULL, NULL);
	if (ret == SOCKET_ERROR) 
	{
		Printf("WSAIoctl(sock, SIO_ADDRESS_LIST_QUERY ...) failed - error code: %u\n", WSAGetLastError());
		return false;
	}
	closesocket(sock);


	// build our list
	for (s32 i=0; i <= (slist.iAddressCount-1); i++) 
	{
		const sockaddr_in* sockAddr = (sockaddr_in *)(slist.Address[i].lpSockaddr);
		net::cIpAddr ip(*sockAddr);
		mLocalIpList.push_back(ip);
	}

	return true;
}// END Refresh



} // namespace net