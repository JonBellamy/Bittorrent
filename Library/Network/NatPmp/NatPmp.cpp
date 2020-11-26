#include "NatPmp.h"


#define USE_THREAD_INIT 1


#include <assert.h>

#include <pthread.h>


#include "Network/WinSock.h"




namespace net {



cNatPmp& NatPmp()
{
	return cNatPmp::Instance();
}

	
cNatPmp::cNatPmp()
: mFoundIgd(false)
, mInitialised(false)
{
#if !USE_JINGLE || UNIT_TESTING
	mUsingGateway = false;
#else
	mUsingGateway = true;
#endif 

	memset(&mNatPmp, 0, sizeof(mNatPmp));
}// END cNatPmp



cNatPmp::~cNatPmp()
{
}// END ~cNatPmp



void* cNatPmp::InitFunc(void* pParam)
{
#if PLATFORM_WIN32
	pthread_win32_thread_attach_np();
#endif

	s32 ret = initnatpmp(&(NatPmp().mNatPmp), false, 0);
	//Printf("initnatpmp() returned %d (%s)\n", r, r?"FAILED":"SUCCESS");
	if(ret == 0)	
	{
		// Check if we can see an igd...
		NatPmp().mFoundIgd = NatPmp().ExternalIp(&(NatPmp().mExternalIp));		
	}
	
	NatPmp().mInitialised = true;


#if PLATFORM_WIN32
	pthread_win32_thread_detach_np();
#endif

	return 0;
}// END InitFunc



void cNatPmp::Init()
{
	if(mInitialised)
	{
		return;
	}

	WinSock().Open();

#if USE_THREAD_INIT
	// This function can block for several seconds, check IsInitialised for when its done
	//boost::thread workerThread(InitFunc); 

	pthread_t thread;
	int rc = pthread_create(&thread, NULL, InitFunc, NULL);
	ASSERT(rc == 0);
#else
	InitFunc(NULL);
#endif
}// END Init



void cNatPmp::DeInit()
{
	closenatpmp(&mNatPmp);
}// END DeInit



bool cNatPmp::ExternalIp(cIpAddr* pIpOut)
{
	if(pIpOut == NULL)
	{
		return false;
	}

	s32 ret;
	natpmpresp_t response;
	int sav_errno;
	fd_set fds;
	struct timeval timeout;
	
	

	ret = sendpublicaddressrequest(&mNatPmp);
	//Printf("sendpublicaddressrequest returned %d (%s)\n", ret, ret==2?"SUCCESS":"FAILED");
	if(ret < 0)
	{
		return false;
	}


	do 
	{
		FD_ZERO(&fds);
		FD_SET(mNatPmp.s, &fds);
		getnatpmprequesttimeout(&mNatPmp, &timeout);
		select(FD_SETSIZE, &fds, NULL, NULL, &timeout);

		ret = readnatpmpresponseorretry(&mNatPmp, &response);
		
		sav_errno = errno;
		//Printf("readnatpmpresponseorretry returned %d (%s)\n", r, r==0?"OK":(r==NATPMP_TRYAGAIN?"TRY AGAIN":"FAILED"));
		if(ret < 0 && ret != NATPMP_TRYAGAIN) 
		{
			//Printf("errno=%d '%s'\n", sav_errno, strerror(sav_errno));
		}
	} 
	while(ret==NATPMP_TRYAGAIN);

	if(ret < 0)
	{
		return false;
	}
		
	*pIpOut = cIpAddr(response.pnu.publicaddress.addr.s_addr);

	// TODO : check that response.type == 0
	//Printf("Public IP address : %s\n", inet_ntoa(response.pnu.publicaddress.addr));
	return true;
}// END ExternalIp



// Return -1 on failure otherwise returns the external port that was mapped
s32 cNatPmp::AddPortMapping(u16 externalPort, u16 internalPort, eProtocol protocol, u32 lifetimeInSeconds)
{
	if(!IsInitialised())
	{
		assert(0);
		return -1;
	}

	if(mFoundIgd == false)
	{
		return -1;
	}

	s32 ret;
	natpmpresp_t response;
	fd_set fds;
	struct timeval timeout;

	ret = sendnewportmappingrequest(&mNatPmp, protocol, internalPort, externalPort, lifetimeInSeconds);
	//Printf("sendnewportmappingrequest returned %d (%s)\n", ret, ret==12?"SUCCESS":"FAILED");
	if(ret < 0)
	{
		return -1;
	}

	do 
	{
		FD_ZERO(&fds);
		FD_SET(mNatPmp.s, &fds);
		getnatpmprequesttimeout(&mNatPmp, &timeout);
		select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
		
		ret = readnatpmpresponseorretry(&mNatPmp, &response);
		//Printf("readnatpmpresponseorretry returned %d (%s)\n",	ret, ret==0?"OK":(ret==NATPMP_TRYAGAIN?"TRY AGAIN":"FAILED"));
	} 
	while(ret==NATPMP_TRYAGAIN);

	if(ret<0) 
	{
		return -1;
	}

	// Return the ACTUAL mapped external port
	Printf("Mapped public port %u protocol %s to local port %u lifetime %u\n", response.pnu.newportmapping.mappedpublicport, response.type == NATPMP_RESPTYPE_UDPPORTMAPPING ? "UDP" : (response.type == NATPMP_RESPTYPE_TCPPORTMAPPING ? "TCP" : "UNKNOWN"), response.pnu.newportmapping.privateport, response.pnu.newportmapping.lifetime);
	return response.pnu.newportmapping.mappedpublicport;
}// END AddPortMapping



bool cNatPmp::DeletePortMapping(u16 externalPort, u16 internalPort, eProtocol protocol)
{
	return (AddPortMapping(externalPort, internalPort, protocol, 0) > 0);
}// END DeletePortMapping



} //namespace net

