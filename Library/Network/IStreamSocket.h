// Jon Bellamy


#ifndef STREAM_SOCKET_H
#define STREAM_SOCKET_H

#include "WinSock.h"
#include "Network/NetSettings.h"
#include "Network/SockAddr.h"
#include "Network/bytestream.h"


namespace net {



class IStreamSocket
{
public:
	IStreamSocket() {}
	virtual ~IStreamSocket() {}

	virtual void operator = (SOCKET s) =0;

    virtual bool OpenAndListen(const cSockAddr& addr, bool bBlocking=false) =0;
	virtual bool OpenAndConnect(const cSockAddr& addr, bool bBlocking=false) =0;

	virtual void SetBlockingState(bool bBlocking) =0;

	virtual bool ConnectionEstablished() const =0;

	virtual bool ProcessIncomingConnection(IStreamSocket& newConnection, cSockAddr& addrOut) =0;

    virtual bool Close() =0;

	virtual s32 Recv(void* pBuffer, u32 bufSize, bool peak=false) const =0;
    virtual s32 Recv(cByteStream* pBuffer, u32 maxRecvSize, bool peak=false) const =0;
    virtual s32 Send(const void* pPacket, u32 packetSize) =0;

	virtual u32 BytesPendingOnInputBuffer() const =0;

	virtual SOCKET GetSocket() const =0;
	virtual void   SetSocket(SOCKET s) =0;

	virtual bool IsOpen() const =0;
	virtual u32 MaxMsgSize() const =0;
};


//////////////////////////////////////////////////////////////////////////





} // namespace net



#endif // TCP_NET_SOCKET_H
