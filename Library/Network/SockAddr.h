// Jon Bellamy 13-03-2006
// Ip Address and port


#ifndef SOCK__ADDR_H
#define SOCK__ADDR_H

#include <string>

#include "Network/IpAddr.h"


namespace net {


class cSockAddr
{

public:
	
	cSockAddr();
	cSockAddr(const cIpAddr& ip, const u16 port);
	cSockAddr(const sockaddr_in& ip, const u16 port);

	bool operator== (const cSockAddr& rhs) const;
	bool operator!= (const cSockAddr& rhs) const { return !(*this == rhs); }

	sockaddr* Get() { return reinterpret_cast<sockaddr *>(&mSockAddr); }
	const sockaddr* Get() const { return reinterpret_cast<const sockaddr *>(&mSockAddr); }
	
	const cIpAddr& Ip() const { return mIp; }
	void Ip(const cIpAddr& rhs);

	u16 Port() const { return mPort; }
	void Port(u16 p);

	// copies the data in mSockAddr into ip and port
	void FromSockAddr();

	std::string AsString() const;

private:

	cIpAddr mIp;
	u16 mPort;
	sockaddr_in mSockAddr;
};

} // namespace net

#endif 