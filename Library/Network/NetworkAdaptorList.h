
#ifndef _NIC_LIST_H
#define _NIC_LIST_H


#if USE_PCH
#include "stdafx.h"
#endif


#include <vector>
#include <assert.h>
#include "Network/IpAddr.h"


namespace net 
{



class cNetworkAdaptorList
{
public:
	cNetworkAdaptorList();

	bool Refresh();

	u32 NumberOfAdaptors() const { return static_cast<u32>(mLocalIpList.size()); }
	const net::cIpAddr& GetAdaptorIp(u32 index) const { assert(index < NumberOfAdaptors()); return mLocalIpList[index]; }

private:

	std::vector<net::cIpAddr> mLocalIpList;
};


extern cNetworkAdaptorList& NetworkAdaptorList();

} // namespace net



#endif  // _NIC_LIST_H