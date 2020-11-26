// Jon Bellamy
// udp socket wrapper

#ifndef UDP_NET_SOCKET_H
#define UDP_NET_SOCKET_H


#include "WinSock.h"
#include "./Network/SockAddr.h"




namespace net {




class cUdpSocket
{
public:

	enum
	{
		DEFAULT_SOCKET_RECV_BUFFER_SIZE = 16 * 1024,
		DEFAULT_SOCKET_SEND_BUFFER_SIZE = 16 * 1024
	};

    cUdpSocket(SOCKET sock = INVALID_SOCKET, u32 recvBufferSize=DEFAULT_SOCKET_RECV_BUFFER_SIZE, u32 sendBufferSize=DEFAULT_SOCKET_SEND_BUFFER_SIZE);
	virtual ~cUdpSocket();

    virtual bool Open(const cSockAddr& localAddrToBindTo, bool bBlocking);

	// a connected udp socket can only receive or send data from the connected peer, we can also get icmp messages now
	virtual bool Connect(const cSockAddr& to);
	bool IsConnected() const;

    void Close();

    bool IsOpen() const;
    u32 MaxMsgSize() const;
	void SetBlockingState(bool bBlocking);
    //cSockAddr   Address() const;

    virtual s32 Recv(void* pBuffer, u32 bufSize, cSockAddr* from = NULL);
    virtual s32 Send(const void* pBuffer, u32 packetSize, const cSockAddr* to = NULL);


	SOCKET GetSocket() const { return mSocket; }

	u32 MaxSegmentSize() const { return mMaxSegmentSize; }

private:

#if SIMULATE_NETWORK_CONDITIONS
	friend class cNetworkConditionSimulator;
#endif

    SOCKET mSocket;

	u32 mMaxSegmentSize;
	bool mIsConnectedToSinglePeer;			// udp socket can be used to send data to multiple peers, not a problem if this is false. You can however connect it to a single peer
	bool mBlocking;

	const u32 SOCKET_RECV_BUFFER_SIZE;
	const u32 SOCKET_SEND_BUFFER_SIZE;
};





//////////////////////////////////////////////////////////////////////////



inline bool cUdpSocket::IsOpen() const
{
	return mSocket != INVALID_SOCKET;
}// END IsOpen



inline bool cUdpSocket::IsConnected() const
{ 
	return mIsConnectedToSinglePeer; 
}// END IsConnected



} // namespace net



#endif // UDP_NET_SOCKET_H
