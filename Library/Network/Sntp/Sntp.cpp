// Simple Network Time Protocol (SNTP)
// RFC 1361


/*
time-a.nist.gov
time-b.nist.gov
time-a.timefreq.bldrdoc.gov
time-b.timefreq.bldrdoc.gov
time-c.timefreq.bldrdoc.gov
utcnist.colorado.edu
time.nist.gov
time-nw.nist.gov
nist1.datum.com
nist1.dc.certifiedtime.com
nist1.nyc.certifiedtime.com
nist1.sjc.certifiedtime.com

more servers here:
http://support.microsoft.com/kb/262680#top
*/


#if USE_PCH
#include "stdafx.h"
#endif

#include "Sntp.h"

#include <assert.h>
#include <time.h>

#include "Network/Dns.h"
#include "Network/SockAddr.h"
#include "Network/NetworkAdaptorList.h"
#include "General/Endianness.h"



namespace net {



cSntp::cSntp()
: mCompleteCb(NULL)
, mCompleteCbParam(NULL)
{
}// END cSntp



cSntp::~cSntp()
{
}// END ~cSntp



void cSntp::Process()
{
	if(mRequestPending)
	{
		s32 bytesRcv;
		sSntpMessage inMessage;
		//memset(&inMessage, 0, sizeof(sSntpMessage));

		if(!mSocket.IsOpen())
		{
			mRequestPending=false;
			if(mCompleteCb)
			{
				mCompleteCb(false, 0, "", mCompleteCbParam);
			}
			return;
		}

		bytesRcv = mSocket.Recv(&inMessage, sizeof(sSntpMessage));
		
		if(bytesRcv > 0)
		{
			assert(bytesRcv == sizeof(sSntpMessage));

			endian_swap(inMessage.mTransmitTimestamp);
			u32 fracPart = static_cast<u32> (inMessage.mTransmitTimestamp & 0x00000000FFFFFFFF);
			u32 intPart = static_cast<u32> ((inMessage.mTransmitTimestamp & 0xFFFFFFFF00000000) >> 32);

			u32 secondsSince1900 = intPart;

			//printf("%d DONE ! MsSince1900 %u\n", bytesRcv, SecondsSince1900);

			mSocket.Close();
			mRequestPending=false;

			if(mCompleteCb)
			{
				mCompleteCb(true, secondsSince1900, FormatDateTime(secondsSince1900), mCompleteCbParam);
			}
		}
	}
}// END Process



bool cSntp::Request(const char* serverName, SntpTimeSyncCompleteCb completeCb, void* cbParam)
{
	// TODO : gets the first network adapter here
	cNetworkAdaptorList nal;
	cIpAddr localIp = nal.GetAdaptorIp(0);

	cSockAddr localSockAddr(localIp, SNTP_PORT);
	//printf("localIp %s\n", localIp.AsString());

	bool ret;

	ret = mSocket.Open(localSockAddr, false);
	//printf("ret %d\n", ret);
	if(!ret)
	{
		return false;
	}

	cIpAddr ip;
	if(!cDns::IpAddressFromDomainName(serverName, 0, &ip))
	{
		return false; 
	}

	cSockAddr sockAddr(ip, SNTP_PORT);
	//printf("ip %s\n", ip.AsString());

	ret = mSocket.Connect(sockAddr);
	//printf("connect %d\n", ret);
	if(!ret)
	{
		return false;
	}

	sSntpMessage message;
	memset(&message, 0, sizeof(sSntpMessage));
	message.mVersionNumber = SNTP_VERSION_NUMBER;
	message.mMode = MODE_CLIENT;

	s32 bytesSent = mSocket.Send(&message, sizeof(sSntpMessage) /*, &sockAddr*/);
	printf("bytesSent %d\n", bytesSent);
	if(bytesSent != sizeof(sSntpMessage))
	{
		return false;
	}

	mCompleteCb = completeCb;
	mCompleteCbParam = cbParam;
	mRequestPending = true;

	return true;
}// END Request



bool cSntp::IsLeapYear(u32 year)
{
	if (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
	{
		return true;
	}
	return false;
}// END IsLeapYear



std::string cSntp::FormatDateTime(u32 seconds)
{
	// crt time functions run from Jan 1 1970, sntp time runs from Jan 1 1900
	// remove the 70 years, remembering leap year days
	u32 seventyYearsInSeconds(60u*60u*24u*365u*70u);
	seconds -= seventyYearsInSeconds;
	for(u32 i=1900; i < 1970; i++)
	{
		if(IsLeapYear(i))
		{
			seconds -= (60*60*24);
		}
	}

	char str[128];
	memset(str, 0, sizeof(str));
	__time32_t t(seconds);
	errno_t err = _ctime32_s(str, sizeof(str), &t);
	assert(err == 0);
	return str;
}// END FormatDateTime



} // namespace net
