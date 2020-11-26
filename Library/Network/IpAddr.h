// Jon Bellamy 13-03-2006
// IP Address Wrapper


#ifndef IP_ADDR_H
#define IP_ADDR_H

#include "WinSock.h"


namespace net {


class cIpAddr
{

public:
	
	// sets to the loop back interface
	cIpAddr();

	cIpAddr(u8 a, u8 b, u8 c, u8 d);

	cIpAddr(const char* str);

	cIpAddr(u32 ip);

	cIpAddr(const sockaddr_in& ip);
	
	const cIpAddr& operator= (const cIpAddr& rhs);
	const cIpAddr& operator= (u32 ip);
	bool operator== (const cIpAddr& rhs) const;
	bool operator!= (const cIpAddr& rhs) const { return !(*this == rhs); }

	const char* AsString() const { return mStr; }
	u32 AsU32() const { return (d << 24) + (c << 16) + (b << 8) + a; }

	u8 GetB1() const { return a; }
	u8 GetB2() const { return b; }
	u8 GetB3() const { return c; }
	u8 GetB4() const { return d; }

	bool IsLan();

	void ToLocal();

	static void PrintAllLocalIPs();

	typedef enum
	{
		IP_ADDR_CLASS_A = 0x00,
		IP_ADDR_CLASS_B = 0x80,
		IP_ADDR_CLASS_C = 0xC0,
		IP_ADDR_CLASS_D = 0xE0,
		IP_ADDR_CLASS_E = 0xF0,
		IP_ADDR_CLASS_ERR = -1
	}IpAddrClass;

	// the class of an ip address is identified by the number of leading 1 bits. Class A has 0, B 1. C 2, D 3, E 4
	IpAddrClass Class() const;

	u32 NetworkId() const;
	u32 HostId() const;

private:

	void ToString();

	u8 a, b, c, d;
	//std::string mStr;
	char mStr[16];
};


} // namespace net

#endif 