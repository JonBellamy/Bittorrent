// Thin wrapper around Mini-Upnp: http://miniupnp.free.fr/
// In here we only deal with the Igd (router) aspects of upnp, that is adding new port mappings, getting the external ip address etc.

#ifndef UPNP_IGD_H
#define UPNP_IGD_H

#include <string>

#include "miniupnp/miniupnpc.h"

#include "General/Singleton.h"
#include "Network/IpAddr.h"



namespace net {


class cUpnpIgd : public cSingleton<cUpnpIgd>
{
private:
	cUpnpIgd();
	cUpnpIgd(const cUpnpIgd& rhs);
	
public:
	~cUpnpIgd();
	friend class cSingleton<cUpnpIgd>;


public:
	void Init();
	void DeInit();

	typedef enum
	{
		TCP=0,
		UDP
	}eProtocol;

	bool ExternalIp(cIpAddr* pIpOut);
	
	// Not supported by all routers
	s32 NumberOfExistingPortMappings() const;

	bool AddPortMapping(u16 externalPort, u16 internalPort, eProtocol protocol, const char* mappingDescription);
	bool DeletePortMapping(u16 externalPort, eProtocol protocol);

	bool ExistingPortMapping(u16 externalPort, eProtocol protocol) const;

	void ListExistingPortMappings();


	bool IsInitialised() const { return mInitialised; }
	bool CanUse() const { return mUsingGateway; }
	bool FoundIgd() const { return mFoundIgd; }
	cIpAddr LocalLanIp() { return cIpAddr(mLanIpAddr.c_str()); }	

private:

	static void* InitFunc(void* pParam);

	void FreeDevices();

	// Will block for up to timeout ms
	void DiscoverUpnpDevices(u32 timeout);

	// Not necessarily an Igd device
	struct UPNPDev* GetUpnpDevice(u32 deviceIndex);

	enum
	{
		NO_IGD_FOUND = 0,
		FOUND_CONNECTED_IGD,
		FOUND_DISCONNECTED_IGD,
		FOUND_UPNP_DEVICE_THAT_IS_NOT_AN_IGD
	};


	bool mUsingGateway;
	bool mInitialised;	

	struct UPNPDev* mDeviceList;
	struct UPNPUrls mUpnpUrls;
	struct IGDdatas mIgdData;
	bool mFreeUrls;
	bool mFoundIgd;

	std::string mLanIpAddr;	
};


extern cUpnpIgd& UpnpIgd();



} // namespace net



#endif // UPNP_IGD_H
