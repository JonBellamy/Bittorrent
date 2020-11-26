// Thin wrapper around libnatpmp: http://miniupnp.free.fr/libnatpmp.html


#ifndef NATPMP_H
#define NATPMP_H

#include <assert.h>
#include <string>

#include "libnatpmp/natpmp.h"

#include "General/Singleton.h"
#include "Network/IpAddr.h"



namespace net {


class cNatPmp : public cSingleton<cNatPmp>
{
private:
	cNatPmp();
	cNatPmp(const cNatPmp& rhs);
	
public:
	~cNatPmp();
	friend class cSingleton<cNatPmp>;


public:
	void Init();
	void DeInit();

	typedef enum
	{
		TCP = NATPMP_PROTOCOL_TCP,
		UDP = NATPMP_PROTOCOL_UDP
	}eProtocol;

	
	// Not supported by all routers
	s32 NumberOfExistingPortMappings() const;

	// Return -1 on failure otherwise returns the external port that was mapped
	s32 AddPortMapping(u16 externalPort, u16 internalPort, eProtocol protocol, u32 lifetimeInSeconds);
	bool DeletePortMapping(u16 externalPort, u16 internalPort, eProtocol protocol);

	//bool ExistingPortMapping(u16 externalPort, eProtocol protocol) const;
	//void ListExistingPortMappings();


	bool IsInitialised() const { return mInitialised; }
	bool CanUse() const { return mUsingGateway; }
	bool FoundIgd() const { return mFoundIgd; }	

	const cIpAddr& ExternalIp() const { assert(mInitialised); return mExternalIp; }

private:

	bool ExternalIp(cIpAddr* pIpOut);

	static void* InitFunc(void* pParam);

	natpmp_t mNatPmp;

	cIpAddr mExternalIp;

	bool mUsingGateway;
	bool mInitialised;
	bool mFoundIgd;
};


extern cNatPmp& NatPmp();



} // namespace net



#endif // NATPMP_H
