// Jon Bellamy 13-03-2006
// IP Address Wrapper


#include "IpAddr.h"

#include <assert.h>
#include <stdio.h>


namespace net {


// sets to the loop back interface
cIpAddr::cIpAddr()
: a(127)
, b(0)
, c(0) 
, d(1)
{
	ToString();
}// END cIpAddr



cIpAddr::cIpAddr(u8 A, u8 B, u8 C, u8 D) :
a(A), 
b(B), 
c(C), 
d(D)
{
	ToString();
}// END cIpAddr



cIpAddr::cIpAddr(const char* str)
{
	u32 b1, b2, b3, b4;
	if(sscanf(str, "%u.%u.%u.%u", &b1, &b2, &b3, &b4) != 4)
	{
		// loop back interface
		a = 127;
		b = 0;
		c = 0;
		d = 1;
	}
	a = static_cast<u8> (b1);
	b = static_cast<u8> (b2);
	c = static_cast<u8> (b3);
	d = static_cast<u8> (b4);
	ToString();
}// END cIpAddr



cIpAddr::cIpAddr(u32 ip)
{
    *this = ip;
}// END cIpAddr



cIpAddr::cIpAddr(const sockaddr_in& ip)
{
    *this = ip.sin_addr.s_addr;
}// END cIpAddr

    
    
const cIpAddr& cIpAddr::operator= (u32 ip)
{
	a =  ip & 0x000000ff;
	b =  (ip & 0x0000ff00)>>8;
	c =  (ip & 0x00ff0000)>>16;
	d =  (ip & 0xff000000)>>24;
	ToString();    
	return *this;
}

    
    
const cIpAddr& cIpAddr::operator= (const cIpAddr& rhs)
{
	a = rhs.a;
	b = rhs.b;
	c = rhs.c;
	d = rhs.d;

	memcpy(mStr, rhs.mStr, sizeof(mStr));

	return *this;
}// END operator=

    

bool cIpAddr::operator== (const cIpAddr& rhs) const
{
	return a == rhs.a && b == rhs.b && c == rhs.c && d == rhs.d;
}// END operator==



void cIpAddr::ToString()
{
	sprintf(mStr, "%u.%u.%u.%u", a, b, c, d);
}// END ToString



// The class of an ip address is identified by the number of leading 1 bits. Class A has 0, B 1. C 2, D 3, E 4
cIpAddr::IpAddrClass cIpAddr::Class() const
{
	// checks top bit
	if((a & 0x80) == IP_ADDR_CLASS_A)
	{
		return IP_ADDR_CLASS_A;
	}

	// top 2 bits
	if((a & 0xC0) == IP_ADDR_CLASS_B)
	{
		return IP_ADDR_CLASS_B;
	}

	// 3 bits
	if((a & 0xE0) == IP_ADDR_CLASS_C)
	{
		return IP_ADDR_CLASS_C;
	}

	// 4 bits
	if((a & 0xF0) == IP_ADDR_CLASS_D)
	{
		return IP_ADDR_CLASS_D;
	}

	// 4 bits
	if((a & 0xF0) == IP_ADDR_CLASS_E)
	{
		return IP_ADDR_CLASS_E;
	}
	assert(0);
	return IP_ADDR_CLASS_ERR;
}// END Class



u32 cIpAddr::NetworkId() const
{
	switch(Class())
	{
	case IP_ADDR_CLASS_A:
		{
			// 1 bit class id, 7 bits network id 24 bits host id
			return (AsU32() & 0x7F000000) >> 24;
		}

	case IP_ADDR_CLASS_B:
		{
			// 2 bits class id, 14 bits network id 16 bits host id
			return (AsU32() & 0x3FFF0000) >> 16;
		}

	case IP_ADDR_CLASS_C:
		{
			// 3 bits class id, 21 bits network id 8 bits host id
			return (AsU32() & 0x1FFFFF00) >> 8;
		}

	case IP_ADDR_CLASS_D:
		{
			// 4 bits class id, 27 bits for the multicast group
			return (AsU32() & 0x0FFFFFFF);
		}

	case IP_ADDR_CLASS_E:
		{
			// 4 bits class id, 27 bits reserved
			return (AsU32() & 0x0FFFFFFF);
		}

	default:
		assert(0);
		return 0;
	}
}// END NetworkId



u32 cIpAddr::HostId() const
{
	switch(Class())
	{
	case IP_ADDR_CLASS_A:
		{
			// 1 bit class id, 7 bits network id 24 bits host id
			return (AsU32() & 0x00FFFFFF);
		}

	case IP_ADDR_CLASS_B:
		{
			// 2 bits class id, 14 bits network id 16 bits host id
			return (AsU32() & 0x0000FFFF);
		}

	case IP_ADDR_CLASS_C:
		{
			// 3 bits class id, 21 bits network id 8 bits host id
			return (AsU32() & 0x000000FF);
		}

	case IP_ADDR_CLASS_D:
		{
			// 4 bits class id, 27 bits for the multicast group
			return (AsU32() & 0x0FFFFFFF);
		}

	case IP_ADDR_CLASS_E:
		{
			// 4 bits class id, 27 bits reserved
			return (AsU32() & 0x0FFFFFFF);
		}

	default:
		assert(0);
		return 0;
	}
}// END HostId


} // namespace net

