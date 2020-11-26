// Jon Bellamy 13-03-2006
// Ip Address and port


#if USE_PCH
#include "stdafx.h"
#endif

#include "SockAddr.h"


namespace net {


cSockAddr::cSockAddr() :
mIp(),
mPort(0xffff)
{
	memset(&mSockAddr, 0, sizeof(sockaddr));
	mSockAddr.sin_family      = AF_INET;					// Internet address family 
	mSockAddr.sin_addr.s_addr = inet_addr(mIp.AsString());	// IP string
	mSockAddr.sin_port        = htons(mPort);				//  port 
}


cSockAddr::cSockAddr(const cIpAddr& ip, const u16 port) :
mIp(ip),
mPort(port)
{
	memset(&mSockAddr, 0, sizeof(sockaddr));				// Zero out structure 
	mSockAddr.sin_family      = AF_INET;					// Internet address family 
	mSockAddr.sin_addr.s_addr = inet_addr(mIp.AsString());	// IP string
	mSockAddr.sin_port        = htons(mPort);				//  port 
}


cSockAddr::cSockAddr(const sockaddr_in& ip, const u16 port) :
mSockAddr(ip),
mIp(ip),
mPort(port)
{
}// END cSockAddr



bool cSockAddr::operator== (const cSockAddr& rhs) const
{
	return	(mIp.AsU32() == rhs.Ip().AsU32() &&
			mPort == rhs.Port());
}// END operator==



void cSockAddr::Ip(const cIpAddr& rhs) 
{ 
	mIp = rhs; 
	mSockAddr.sin_addr.s_addr = inet_addr(mIp.AsString());	// IP string
}// END Ip



void cSockAddr::Port(u16 p)
{
	mPort = p;
	mSockAddr.sin_port = htons(mPort);
}// END Port



// copies the data in mSockAddr into ip and port
void cSockAddr::FromSockAddr()
{
	mIp = cIpAddr(mSockAddr);
	mPort = ntohs(mSockAddr.sin_port);
}// END FromSockAddr



std::string cSockAddr::AsString() const
{
	char sz[64];
	sprintf(sz, "%s:%u", Ip().AsString(), Port());
	return std::string(sz);
}// END AsString


} // namespace net