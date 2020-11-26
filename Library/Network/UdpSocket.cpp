// Jon Bellamy
// udp socket wrapper


#if USE_PCH
#include "stdafx.h"
#endif

#include "UdpSocket.h"

#include <assert.h>




namespace net {



cUdpSocket::cUdpSocket(SOCKET sock, u32 recvBufferSize, u32 sendBufferSize)
: SOCKET_RECV_BUFFER_SIZE(recvBufferSize)
, SOCKET_SEND_BUFFER_SIZE(sendBufferSize)
, mSocket(sock)
, mIsConnectedToSinglePeer(false)
, mMaxSegmentSize(0)
, mBlocking(true)
{
}



cUdpSocket::~cUdpSocket()
{
}// END ~cUdpSocket



bool cUdpSocket::Open(const cSockAddr& localAddrToBindTo, bool bBlocking)
{    
    // make sure winsock is open
    WinSock().Open();

    // close the socket if necessary
    Close();

    // create a UDP (datagram) socket
    mSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mSocket == INVALID_SOCKET)
    {
        Printf("cUdpSocket::Open : socket: %s", strerror(errno));
        return false;
    }

	/*
    // set to non-blocking
    unsigned long trueval = true;
	if (ioctlsocket(mSocket, FIONBIO, &trueval))
    {
        Printf("cUdpSocket::Open : ioctl FIONBIO: %s", strerror(errno));
        Close();
        return false;
    }
	*/
	SetBlockingState(bBlocking);

	// set buffer size's
	if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&SOCKET_RECV_BUFFER_SIZE, sizeof(SOCKET_RECV_BUFFER_SIZE)))
	{
		Printf("Error setting socket buffer size\n");
		return false;
	}
	if (setsockopt(mSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&SOCKET_SEND_BUFFER_SIZE, sizeof(SOCKET_SEND_BUFFER_SIZE)))
	{
		Printf("Error setting socket buffer size\n");
		return false;
	}

    // bind to adapter & port
	if (bind(mSocket, localAddrToBindTo.Get(), sizeof(*(localAddrToBindTo.Get()))))
    {
		int error = WSAGetLastError();
		switch(error)
		{
		case WSANOTINITIALISED:
			Printf("WSANOTINITIALISED\n");
			break;
		case WSAENETDOWN:
			Printf("WSAENETDOWN\n");
			break;
		case WSAEACCES:
			Printf("WSAEACCES\n");
			break;
		case WSAEADDRINUSE:
			Printf("WSAEADDRINUSE\n");
			break;
		case WSAEADDRNOTAVAIL:
			Printf("WSAEADDRNOTAVAIL\n");
			break;
		case WSAEFAULT:
			Printf("WSAEFAULT\n");
			break;
		case WSAEINPROGRESS:
			Printf("WSAEINPROGRESS\n");
			break;
		case WSAEINVAL:
			Printf("WSAEINVAL\n");
			break;
		case WSAENOBUFS:
			Printf("WSAENOBUFS\n");
			break;
		case WSAENOTSOCK:
			Printf("\n");
			break;
		}


        Printf("cUdpSocket::Open : bind: %s\n", strerror(errno));
        Close();
		return false;
    }

	/*
    if (broadcast)
    {
        if (setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, 
                        (const char*)&trueval, sizeof(trueval)))
        {
            Printf("cUdpSocket::Open : setsockopt : %s", strerror(errno));
            Close();
            return false;
        }
    }
	*/


	// get the max segment size
	char szSize[16];
	s32 size = sizeof(szSize);
	if(getsockopt(mSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, szSize, &size) != 0)
	{
		return false;
	}
	memcpy(&mMaxSegmentSize, szSize, size);

	return true;
}// END Open



// a connected udp socket can only receive or send data from the connected peer, we can also get icmp messages now
bool cUdpSocket::Connect(const cSockAddr& to)
{
	s32 ret = connect(mSocket, to.Get(), sizeof(*(to.Get())));

	mIsConnectedToSinglePeer = (ret == 0);

	if(mIsConnectedToSinglePeer)
	{	
		return true;
	}
	else
	{
		int err = WSAGetLastError();
		Printf("ERROR : cUdpSocket::Connect : %s", strerror(err));  
		return false;
	}
}// END Connect



void cUdpSocket::Close()
{
    if (mSocket != INVALID_SOCKET)
    {
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
}// END Close



s32 cUdpSocket::Recv(void* pBuffer, u32 bufSize, cSockAddr* from)
{
    if(IsOpen() == false)
	{
		return -1;
	}

	int ret;

	if(from)
	{
		int fromlen = sizeof(*from);

		ret = recvfrom(mSocket, (char*)pBuffer, bufSize, 0, from->Get(), &fromlen);

		// refresh the recv address
		from->FromSockAddr();
	}
	else
	{
		assert(mIsConnectedToSinglePeer);
		ret = recv(mSocket, (char*)pBuffer, bufSize, 0);
	}

    if (ret < 0) 
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return 0;

        if (err == WSAEMSGSIZE) 
        {
            Printf("Warning: Oversize packet\n");
        }

        if (err == WSAECONNRESET) 
        {
            Printf("cUdpSocket::Recv : Connection reset\n");
        }

        Printf("cUdpSocket::Recv : %s\n", strerror(err));  
    }

    return ret;
}// END Recv




s32 cUdpSocket::Send(const void* pBuffer, u32 packetSize, const cSockAddr* to)
{
    assert(IsOpen());

	assert(packetSize <= MaxSegmentSize());

	s32 ret;

	if(to)
	{
		ret = sendto(mSocket, (const char*)pBuffer, packetSize, 0, to->Get(), sizeof(*to));
	}
	else
	{
		assert(mIsConnectedToSinglePeer);
		ret = send(mSocket, (const char*)pBuffer, packetSize, 0);
	}
	
	if (ret < 0) 
	{
		int err = WSAGetLastError();

		switch (err)
		{
		case WSAEWOULDBLOCK:
			return 0;	

		case WSAECONNRESET:
			//Printf("UDP Socket : The socket was reset by the remote side executing a hard or abortive close.\n");
			break;

		default:
			//Printf("cUdpSocket::Send : %s\n", strerror(err));
			break;
		}
	}

    return ret;
}// END Send



u32 cUdpSocket::MaxMsgSize() const
{
    assert(IsOpen());

    u32 msgsize;
    int ssize = sizeof(msgsize);
    if (getsockopt(mSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&msgsize, &ssize))
    {
        Printf("cUdpSocket::MaxSendSize : %s\n", strerror(WSAGetLastError()));
        return 0;
    }

    return msgsize;
}// END MaxMsgSize



void cUdpSocket::SetBlockingState(bool bBlocking)
{
	mBlocking = bBlocking;
	unsigned long trueval = !bBlocking;
	s32 ret = ioctlsocket(mSocket, FIONBIO, &trueval);
	if (ret != 0)
	{
		int err = WSAGetLastError();
		Printf("SetBlockingState failed. err = %d\n", err);
		assert(0);
		Close();
	}
}// END SetBlockingState



/*
cSockAddr cUdpSocket::Address() const
{
    cSockAddr addr;
    int addrlen = sizeof(addr.Get());
    if (getsockname(mSocket, addr.Get(), &addrlen))
    {
        Printf("cUdpSocket::Address : %s\n", strerror(WSAGetLastError()));
    }

    return addr;
}// END Address
*/



} //namespace net

