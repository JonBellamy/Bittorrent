// Jon Bellamy
// tcp socket wrapper

#if USE_PCH
#include "stdafx.h"
#endif

#include "TcpSocket.h"

#include "Network/bytestream.h"

#include <assert.h>


#define CONNECTED_TEST_ZERO_BYTE_READ 1


namespace net {


cTcpSocket::cTcpSocket(SOCKET sock, u32 sendBufferSize, u32 recvBufferSize)
: mSocket(sock)
, mListening(false)
, mBlocking(true)
, mDontClose(false)
, SOCKET_RECV_BUFFER_SIZE(recvBufferSize)
, SOCKET_SEND_BUFFER_SIZE(sendBufferSize)
{
}// END cTcpSocket



cTcpSocket::cTcpSocket(const cTcpSocket& rhs)
{
	*this = rhs;
}// END cTcpSocket



cTcpSocket::~cTcpSocket()
{
	if(!mDontClose)
	{
		Close();
	}
}// END ~cTcpSocket



void cTcpSocket::operator= (SOCKET s)
{
	assert(s != INVALID_SOCKET);

	mSocket = s;
	mListening = false;			// TODO : you can't know this, or if its blocking
}// END operator=



const cTcpSocket& cTcpSocket::operator= (const cTcpSocket& rhs)
{
	if(mSocket != INVALID_SOCKET)
	{
		Close();
	}

	assert(rhs.mSocket != INVALID_SOCKET);
	mSocket = rhs.mSocket;
	mListening = rhs.mListening;
	mBlocking = rhs.mBlocking;
	mDontClose = rhs.mDontClose;

	SOCKET_RECV_BUFFER_SIZE = rhs.SOCKET_RECV_BUFFER_SIZE;
	SOCKET_SEND_BUFFER_SIZE = rhs.SOCKET_SEND_BUFFER_SIZE;
	
	return *this;
}// END operator=



bool cTcpSocket::operator== (const cTcpSocket& rhs) const
{
	return mSocket == rhs.mSocket;
}// END operator==



bool cTcpSocket::OpenAndListen(const cSockAddr& addr, bool bBlocking)
{    
    // make sure winsock is open
    WinSock().Open();

	if(IsOpen())
	{
		assert(0);
		return false;
	}

    mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == INVALID_SOCKET)
    {
        Printf("cTcpSocket::Open : socket: %s", strerror(errno));
        return false;
    }

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
	if (bind(mSocket, addr.Get(), sizeof(*(addr.Get()))))
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


        Printf("cTcpSocket::Open : bind: %s\n", strerror(errno));
        Close();
		return false;
    }


	// Maximum outstanding connection requests 
	const s32 MAXPENDING = 4;
	int ret = listen(mSocket, MAXPENDING);
	if (ret)
	{
		int error = WSAGetLastError();
		switch(error)
		{

		case WSAEISCONN:
			Printf("Socket is already connected\n");
			return false; 

		case WSAEOPNOTSUPP:
			Printf("The referenced socket is not of a type that supports the listen operation.\n");
			return false; 

		default:
			assert(0);
		}
	}

	mListening = true;

	Printf("TCP connection listener opened and listening on %s port %d\n", addr.Ip().AsString(), addr.Port());
	return true;
}// END OpenAndListen



// returns true if the connection process has started, note that it does not mean that the sockets are yet connected
bool cTcpSocket::OpenAndConnect(const cSockAddr& addr, bool bBlocking)
{
	assert(addr.Ip() != cIpAddr(127,0,0,1));

	WinSock().Open();
	
	if(IsOpen())
	{
		assert(0);
		return false;
	}

	mListening = false;

	mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocket == INVALID_SOCKET)
	{
		Printf("cTcpSocket::Open : socket: %s\n", strerror(errno));
		return false;
	}


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

	Printf("Connecting to %s Port %d\n", addr.Ip().AsString(), addr.Port());
	
	// Connect to server 	
	int ret = connect(mSocket, addr.Get(), sizeof(*(addr.Get())));
	if (ret < 0)
	{
		int err = WSAGetLastError();
		
		switch(err)
		{
		case WSAEWOULDBLOCK:
			assert(!mBlocking);
			break;

		case WSANOTINITIALISED:
		case WSAENETDOWN:
		case WSAEADDRINUSE:
		case WSAEINTR:
		case WSAEINPROGRESS:
		case WSAEALREADY:
		case WSAEADDRNOTAVAIL:
		case WSAEAFNOSUPPORT:
		case WSAECONNREFUSED:
		case WSAEFAULT:
		case WSAEINVAL:
		case WSAEISCONN:
		case WSAENETUNREACH:
		case WSAENOBUFS:
		case WSAENOTSOCK:
		case WSAEACCES:
		case WSAETIMEDOUT:
			Printf("NETWORK ERROR on connect()\n");
			return false;

		default:
			Printf("Connection Failed \n");
			return false;
		}
	}

	//Printf("TCP socket connected.\n");

	return true;
}// END OpenAndConnect



// Tests if accept will return us a new connection
bool cTcpSocket::IsNewConnectionPending() const
{
	assert(mListening);

	fd_set set;
	set.fd_count = 1;
	set.fd_array[0] = mSocket;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// If the socket is currently in the listen state, it will be marked as readable if an incoming connection request has been received such 
	// that an accept is guaranteed to complete without blocking.
	if (select(0, &set, NULL, NULL, &timeout) == 1)
	{
		return true;
	}
	return false;
}// END IsNewConnectionPending



bool cTcpSocket::ProcessIncomingConnection(IStreamSocket& newConnection, cSockAddr& addrOut)
{
	if (!mListening)
	{
		// incoming connection on a socket thats not listening
		assert(0);
		return false;
	}


	struct sockaddr_in clientAddr;			// Client address 
	int clientLen = sizeof(sockaddr_in);	// Length of client address data structure 
	SOCKET sock = accept(mSocket, (struct sockaddr *) &clientAddr, &clientLen);	
	
	// copy the new clients address
	addrOut = cSockAddr(clientAddr, ntohs(clientAddr.sin_port));
	
	if (sock == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		switch (err)
		{
		case WSAEWOULDBLOCK:
			return false;

		default:
			Printf("accept() failed. err = %d\n", err);
			assert(0);
			return false;
		}
	}
	else
	{
		//Printf("Incoming connection from %s sock %X\n", addrOut.Ip().AsString(), sock);
	}

	newConnection.SetSocket(sock);

	// connected to a client!

	// set the blocking state to match the listeners
	newConnection.SetBlockingState(mBlocking);

	// set buffer size's
	if (setsockopt(newConnection.GetSocket(), SOL_SOCKET, SO_RCVBUF, (const char*)&SOCKET_RECV_BUFFER_SIZE, sizeof(SOCKET_RECV_BUFFER_SIZE)))
	{
		Printf("Error setting socket buffer size\n");
		return false;
	}
	if (setsockopt(newConnection.GetSocket(), SOL_SOCKET, SO_SNDBUF, (const char*)&SOCKET_SEND_BUFFER_SIZE, sizeof(SOCKET_SEND_BUFFER_SIZE)))
	{
		Printf("Error setting socket buffer size\n");
		return false;
	}

	//Printf("Handling incoming TCP connection %s ... Connected\n", inet_ntoa(clientAddr.sin_addr));

	return true;
}// END ProcessIncomingConnection



bool cTcpSocket::Close()
{
    if (mSocket != INVALID_SOCKET)
    {
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
		return true;
    }
	return false;
}// END Close



s32 cTcpSocket::Send(const void* pPacket, u32 packetSize)
{
	// NB : This is deliberately shorting any virtual method call for ConnectionEstablished.
	if(!IsOpen() || !cTcpSocket::ConnectionEstablished())
	{
		//Printf("Send Failed IsOpen %d ConnectionEstablished %d\n", /*AddressLocal().Ip().AsString(),*/ IsOpen(), cTcpSocket::ConnectionEstablished());
		return 0;
	}

	s32 ret = send(mSocket, (const char*)pPacket, packetSize, 0);

	if (ret < 0) 
	{
		int err = WSAGetLastError();
		switch (err)
		{
		case WSAEWOULDBLOCK:
			return 0;	

			// peer has disconnected 
		case WSAECONNRESET:
			//Printf("send disconnection : The virtual circuit was reset by the remote side executing a hard or abortive close.\n");
			Close();
			break;

		default:
			//Printf("cTcpSocket::Send : %s\n", strerror(err));
			break;
		}
	}

	return ret;
}// END Send



s32 cTcpSocket::Recv(void* pBuffer, u32 bufSize, bool peak) const
{
	// NB : This is deliberately shorting any virtual method call for ConnectionEstablished.
	if(!IsOpen() || !cTcpSocket::ConnectionEstablished())
	{
		//Printf("Recv Failed IsOpen %d ConnectionEstablished %d\n", /*AddressLocal().Ip().AsString(),*/ IsOpen(), cTcpSocket::ConnectionEstablished());
		return -1;
	}

	int flags = 0;
	if(peak)
	{
		flags = MSG_PEEK;
	}

    int ret = recv(mSocket, (char*)pBuffer, bufSize, flags);

    if (ret < 0) 
    {
        int err = WSAGetLastError();		
		switch(err)
		{
		case WSAEWOULDBLOCK:
			return 0;

		case WSAEMSGSIZE:
			Printf("cTcpSocket::Recv : Oversize packet\n");
			break;

		case WSAECONNRESET:
			//Printf("cTcpSocket::Recv : Connection reset\n");
			break;

		case WSAECONNABORTED:
			//Printf("cTcpSocket::Recv : Software caused connection abort.\n");
			break;

		case WSAENOBUFS:
			Printf("cTcpSocket::Recv : No buffer space available. \n");
			break;

		default:
			//Printf("cTcpSocket::Recv : %s\n", strerror(err)); 
			break;
		}
    }

    return ret;
}// END Recv



// this should work for cSslSocket's as well
s32 cTcpSocket::Recv(cByteStream* pBuffer, u32 maxRecvSize, bool peak) const
{
	assert(maxRecvSize > 0);
	SizeByteStreamForRecv(pBuffer, maxRecvSize);

	// manually copy the bytes into the byte stream and update its size
	s32 bytesRead = Recv(pBuffer->Data() + pBuffer->Size(), maxRecvSize, peak);
	
	if(bytesRead > 0)
	{
		pBuffer->BytesStreamedManually(bytesRead);
	}
	
	return bytesRead;
}// END Recv



void cTcpSocket::SizeByteStreamForRecv(cByteStream* pBuffer, u32 maxRecvSize) const
{
	u32 pendingBytes = BytesPendingOnInputBuffer();
	if(pendingBytes == 0)
	{
		return;
	}

	u32 bytesToRead = min(pendingBytes, maxRecvSize);
	if(bytesToRead > pBuffer->FreeSpace())
	{
		pBuffer->Resize(pBuffer->Capacity() + bytesToRead, true);
	}
}// END SizeByteStreamForRecv



// this call is only valid if the socket was previously in the established state
bool cTcpSocket::ConnectionClosing() const
{
	if(mSocket == INVALID_SOCKET)
	{
		return false;
	}

	fd_set set;
	set.fd_count = 1;
	set.fd_array[0] = mSocket;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// test that we can read but not write data to the socket
	if (select(0, &set, NULL, NULL, &timeout) == 1 && select(0, NULL, &set, NULL, &timeout) <= 0)
	{
		return true;
	}
	return false;
}// END ConnectionClosing



bool cTcpSocket::ConnectionEstablished() const
{
#if CONNECTED_TEST_ZERO_BYTE_READ
	int ret = send(mSocket, NULL, 0, 0);
	if(IsOpen() == false || ret != 0)
	{
		int err = WSAGetLastError();
		if(err != WSAEWOULDBLOCK)
		{
			//Printf("SetBlockingState failed. err = %d\n", err);
			return false;
		}
	}
	return true;
#else
	if(mSocket == INVALID_SOCKET)
	{
		return false;
	}
	fd_set set;
	set.fd_count = 1;
	set.fd_array[0] = mSocket;
	timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	// test if we can read & write data to the socket, ie that it is connected
	if (select(0, &set, &set, NULL, &timeout) == 1)
	{
		return true;
	}
	return false;
#endif
}// END ConnectionEstablished



void cTcpSocket::SetBlockingState(bool bBlocking)
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



void cTcpSocket::SetUseNagle(bool b)
{
	// Nagle algorithm (potentially 200ms delay before the send will occur)
	const s8 on = b;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}// END SetUseNagle



u32 cTcpSocket::BytesPendingOnInputBuffer() const
{
	if(!IsOpen())
	{
		return 0;
	}

	u32 numBytes;
	if(ioctlsocket(mSocket, FIONREAD, reinterpret_cast<unsigned long*>(&numBytes)) != 0)
	{
		return 0;
	}

	return numBytes;
}// END BytesPendingOnInputBuffer



u32 cTcpSocket::MaxMsgSize() const
{
    assert(IsOpen());

    u32 msgsize;
    int size = sizeof(msgsize);
    if (getsockopt(mSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&msgsize, &size))
    {
		const char* err = strerror(WSAGetLastError());
        Printf("cTcpSocket::MaxSendSize : %s\n", err);
		assert(0);
        return 0;
    }

    return msgsize;
}// END MaxMsgSize



cSockAddr cTcpSocket::AddressLocal() const
{
    cSockAddr addr;
    int addrlen = sizeof(*(addr.Get()));
    if (getsockname(mSocket, addr.Get(), &addrlen))
    {
		int errCode = WSAGetLastError();
		const char* err = strerror(errCode);
        Printf("cTcpSocket::AddressLocal : %s\n", err);
//		assert(0);
    }
	addr.FromSockAddr();
    return addr;
}// END AddressLocal



cSockAddr cTcpSocket::AddressRemote() const
{
	cSockAddr addr;
	int addrlen = sizeof(*(addr.Get()));
	if (getpeername(mSocket, addr.Get(), &addrlen))
	{
		int errCode = WSAGetLastError();
		const char* err = strerror(errCode);
		Printf("cTcpSocket::AddressRemote : %s\n", err);
		assert(0);
	}
	addr.FromSockAddr();
	return addr;
}// END AddressRemote


} //namespace net

