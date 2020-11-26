// Jon Bellamy
// tcp socket wrapper

#ifndef TCP_NET_SOCKET_H
#define TCP_NET_SOCKET_H


#include "Network/NetSettings.h"
#include "WinSock.h"
#include "./Network/SockAddr.h"
#include "Network/IStreamSocket.h"


namespace net {


class cTcpSocket : public IStreamSocket
{
public:

	enum
	{
		DEFAULT_SOCKET_SEND_BUFFER_SIZE = 1024 * 64,
		DEFAULT_SOCKET_RECV_BUFFER_SIZE = 1024 * 64,
	};

    cTcpSocket(SOCKET sock = INVALID_SOCKET, u32 sendBufferSize=DEFAULT_SOCKET_SEND_BUFFER_SIZE, u32 recvBufferSize=DEFAULT_SOCKET_RECV_BUFFER_SIZE);
	cTcpSocket(const cTcpSocket& rhs);
	virtual ~cTcpSocket();

	void operator= (SOCKET s);
	const cTcpSocket& operator= (const cTcpSocket& rhs);
	bool operator==  (const cTcpSocket& rhs) const;


    bool OpenAndListen(const cSockAddr& addr, bool bBlocking=false);
	bool OpenAndConnect(const cSockAddr& addr, bool bBlocking=false);

	bool IsNewConnectionPending() const;
	virtual bool ProcessIncomingConnection(IStreamSocket& newConnection, cSockAddr& addrOut);

    virtual bool Close();

    s32 Recv(void* pBuffer, u32 bufSize, bool peak=false) const;
	s32 Recv(cByteStream* pBuffer, u32 maxRecv, bool peak=false) const;
    s32 Send(const void* pPacket, u32 packetSize);

	bool IsOpen() const;

	bool ConnectionEstablished() const;
	bool ConnectionClosing() const;

	void SetBlockingState(bool bBlocking);
	u32 BytesPendingOnInputBuffer() const;
	void SetUseNagle(bool b);

	u32 MaxMsgSize() const;
	cSockAddr AddressLocal() const;
	cSockAddr AddressRemote() const;

	SOCKET GetSocket() const { return mSocket; }
	void SetSocket(SOCKET s) { mSocket = s; }

	void SetDontClose(bool b) { mDontClose = b; }

protected:

	void SizeByteStreamForRecv(cByteStream* pBuffer, u32 maxRecvSize) const;

    SOCKET      mSocket;
	bool		mListening;
	bool		mBlocking;
	bool		mDontClose;		// will the socket close when the destructor is called

	// socket buffer sizes
	/*const*/ s32 SOCKET_RECV_BUFFER_SIZE;
	/*const*/ s32 SOCKET_SEND_BUFFER_SIZE;
};



inline bool cTcpSocket::IsOpen() const
{
	return mSocket != INVALID_SOCKET;
}



} // namespace net



#endif // TCP_NET_SOCKET_H
