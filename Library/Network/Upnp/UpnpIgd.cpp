#include "UpnpIgd.h"

#define USE_THREAD_INIT 0


#include <assert.h>

#if USE_THREAD_INIT
#include <pthread.h>
#endif

#include "Network/WinSock.h"

#include "miniupnp/miniwget.h"
#include "miniupnp/upnpcommands.h"
#include "miniupnp/upnperrors.h"



namespace net {



cUpnpIgd& UpnpIgd()
{
	return cUpnpIgd::Instance();
}

	
cUpnpIgd::cUpnpIgd()
: mDeviceList(NULL)
, mFreeUrls(false)
, mInitialised(false)
{
#if UNIT_TESTING
	mUsingGateway = false;
#else
	mUsingGateway = true;
#endif 
}// END cUpnpIgd



cUpnpIgd::~cUpnpIgd()
{
}// END ~cUpnpIgd



void* cUpnpIgd::InitFunc(void* pParam)
{
	UpnpIgd().DiscoverUpnpDevices(2000);
	if(UpnpIgd().mDeviceList)
	{
		char localLanIpAddr[64];

		// Hmm so we only can discover one igd per nic?
		int ret = UPNP_GetValidIGD(UpnpIgd().mDeviceList, &(UpnpIgd().mUpnpUrls), &(UpnpIgd().mIgdData), localLanIpAddr, sizeof(localLanIpAddr));
		if(ret != 0)
		{
			UpnpIgd().mLanIpAddr = localLanIpAddr;
			UpnpIgd().mFreeUrls = true;
			//printf("Local LAN ip address : %s\n", lanaddr);
		}
		
		if(ret == FOUND_CONNECTED_IGD)
		{
			//Printf("Found IGD via UPnP!\n");
			UpnpIgd().mFoundIgd = true;
		}		
	}
	else
	{
		//Printf("No IGD UPnP Device found on the network !\n");
	}

	UpnpIgd().mInitialised = true;

	return 0;
}// END InitFunc



void cUpnpIgd::Init()
{
	if(mInitialised)
	{
		return;
	}

	WinSock().Open();

#if USE_THREAD_INIT
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, InitFunc, NULL);
	ASSERT(rc == 0);
#else
	InitFunc(NULL);
#endif
}// END Init



void cUpnpIgd::DeInit()
{
	FreeDevices();
}// END DeInit



void cUpnpIgd::FreeDevices()
{
	if(mDeviceList != NULL)
	{
		freeUPNPDevlist(mDeviceList);
		mDeviceList=NULL;
	}

	if(mFreeUrls)
	{
		FreeUPNPUrls(&mUpnpUrls);
		mFreeUrls = false;
	}
}// END FreeDevices



// Will block for up to timeout ms
void cUpnpIgd::DiscoverUpnpDevices(u32 timeout)
{
	FreeDevices();
	assert(mDeviceList == NULL);
	mDeviceList = upnpDiscover(timeout, NULL, NULL, 0);
}// END DiscoverUpnpDevices



// Not necessarily an Igd device
struct UPNPDev* cUpnpIgd::GetUpnpDevice(u32 deviceIndex)
{
	struct UPNPDev * device;
	u32 i=0;
	for(device = mDeviceList; device; device = device->pNext)
	{
		if(deviceIndex == i)
		{
			return device;
		}
		i++;
	}
	return NULL;
}// END GetUpnpDevice



bool cUpnpIgd::ExternalIp(cIpAddr* pIpOut)
{
	if(!IsInitialised())
	{
		assert(0);
		return false;
	}

	if(pIpOut == NULL)
	{
		assert(0);
		return false;
	}

	if(mFoundIgd == false)
	{
		return false;
	}
	
	char externalIPAddress[16];
	memset(externalIPAddress, 0, sizeof(externalIPAddress));

	int ret = UPNP_GetExternalIPAddress(mUpnpUrls.controlURL, mIgdData.first.servicetype, externalIPAddress);
	if( ret == UPNPCOMMAND_SUCCESS &&
		externalIPAddress[0])
	{
		*pIpOut = cIpAddr(externalIPAddress);
		Printf("Get external Ip succeeded: %s\n", externalIPAddress);
		return true;
	}

	Printf("GetExternalIPAddress failed.\n");	
	pIpOut = NULL;
	return false;
}// END ExternalIp



// Not supported by all routers
s32 cUpnpIgd::NumberOfExistingPortMappings() const
{
	if(!IsInitialised())
	{
		assert(0);
		return 0;
	}

	if(mFoundIgd == false)
	{
		return 0;
	}

	u32 num;
	s32 ret = UPNP_GetPortMappingNumberOfEntries(mUpnpUrls.controlURL, mIgdData.first.servicetype, &num);
	if(ret != 0)
	{
		return -1;
	}
	return num;
}// END NumberOfExistingPortMappings



bool cUpnpIgd::AddPortMapping(u16 externalPort, u16 internalPort, eProtocol protocol, const char* mappingDescription)
{
	if(!IsInitialised())
	{
		assert(0);
		return false;
	}

	if(mFoundIgd == false)
	{
		return false;
	}
	
	char szExternalPort[8];
	char szIternalPort[8];
	sprintf(szExternalPort, "%u", externalPort);
	sprintf(szIternalPort, "%u", internalPort);
	std::string strProtocol = (protocol == cUpnpIgd::TCP) ? "TCP" : "UDP";

	char intClient[16];
	char intPort[6];
	int ret;

	// Don't add if the mapping already exists.
	// NB: You don't have to bail here, the igd will add the mapping even if one already exists but it will pick another port.
	if(ExistingPortMapping(externalPort, protocol))
	{
		Printf("Mapping already exists for port %u\n", externalPort);
		return false;
	}

	
	ret = UPNP_AddPortMapping(mUpnpUrls.controlURL, mIgdData.first.servicetype, szExternalPort, szIternalPort, mLanIpAddr.c_str(), mappingDescription, strProtocol.c_str(), NULL);
	if(ret != UPNPCOMMAND_SUCCESS)
	{
		Printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n", szExternalPort, szIternalPort, mLanIpAddr.c_str(), ret, strupnperror(ret));
		return false;
	}

	ret = UPNP_GetSpecificPortMappingEntry(mUpnpUrls.controlURL, mIgdData.first.servicetype, szExternalPort, strProtocol.c_str(), intClient, intPort);
	if(ret != UPNPCOMMAND_SUCCESS)
	{
		printf("GetSpecificPortMappingEntry() failed with code %d (%s)\n", ret, strupnperror(ret));
		return false;
	}
	
	if(intClient[0]) 
	{
		Printf("external %s port %s is redirected to internal %s:%s\n", strProtocol.c_str(), szExternalPort, intClient, intPort);		
	}
	return true;
}// END AddPortMapping



bool cUpnpIgd::DeletePortMapping(u16 externalPort, eProtocol protocol)
{
	if(!IsInitialised())
	{
		assert(0);
		return false;
	}

	if(mFoundIgd == false)
	{
		return false;
	}

	char szExternalPort[8];
	sprintf(szExternalPort, "%u", externalPort);
	std::string strProtocol = (protocol == cUpnpIgd::TCP) ? "TCP" : "UDP";
	int ret = UPNP_DeletePortMapping(mUpnpUrls.controlURL, mIgdData.first.servicetype, szExternalPort, strProtocol.c_str(), NULL);
	return ret == 0;
}// END DeletePortMapping



bool cUpnpIgd::ExistingPortMapping(u16 externalPort, eProtocol protocol) const
{
	if(!IsInitialised())
	{
		assert(0);
		return false;
	}

	std::string strProtocol = (protocol == cUpnpIgd::TCP) ? "TCP" : "UDP";
	char szExternalPort[8];
	sprintf(szExternalPort, "%u", externalPort);

	char intClient[16];
	char intPort[6];

	s32 ret = UPNP_GetSpecificPortMappingEntry(mUpnpUrls.controlURL, mIgdData.first.servicetype, szExternalPort, strProtocol.c_str(), intClient, intPort);
	if(ret == UPNPCOMMAND_SUCCESS)
	{
		return true;
	}
	return false;
}// END ExistingPortMapping



void cUpnpIgd::ListExistingPortMappings()
{
	if(!IsInitialised())
	{
		assert(0);
		return;
	}

	if(mFoundIgd == false)
	{
		return;
	}

	int r;
	int i = 0;
	char index[32];
	char intClient[16];
	char intPort[6];
	char extPort[6];
	char protocol[4];
	char desc[80];
	char enabled[6];
	char rHost[64];
	char duration[16];
	cIpAddr externalIp;
	ExternalIp(&externalIp);
	do 
	{
		sprintf(index, "%d", i);
		rHost[0] = '\0'; enabled[0] = '\0';
		duration[0] = '\0'; desc[0] = '\0';
		extPort[0] = '\0'; intPort[0] = '\0'; intClient[0] = '\0';
		r = UPNP_GetGenericPortMappingEntry(mUpnpUrls.controlURL, mIgdData.first.servicetype, index, extPort, intClient, intPort, protocol, desc, enabled, rHost, duration);
		if(r==0)
		{
			Printf("%02d - %s %s:%s->%s:%s\tenabled=%s leaseDuration=%s desc='%s' rHost='%s'\n", i, protocol, externalIp.AsString(), extPort, intClient, intPort, enabled, duration, desc, rHost);
		}		   
		else
		{
			Printf("GetGenericPortMappingEntry() returned %d (%s)\n", r, strupnperror(r));
		}
		i++;
	} 
	while(r==0);
}// END ListExistingPortMappings



} //namespace net

