#if USE_PCH
#include "stdafx.h"
#endif

#include "WhoIs.h"

#include "Network/nds/Core/Dns_NDS.h"


namespace net {



// whois.internic.net
// whois.arin.net 

// whois.nic.uk							<< .uk
// whois.nic.gov						<< .gov
// rs.internic.net						<< .net .com
// whois.educause.net					<< .edu
// whois.publicinterestregistry.net		<< .org	



cWhoIs::cWhoIs()
: mByteStream(DEFAULT_BUFFER_SIZE)
, mRequestPending(false)
, mCb(NULL)
, mpParam(NULL)
{
}// END cWhoIs



cWhoIs::~cWhoIs()
{
	mSocket.Close();
	mByteStream.Clear();
}// END ~cWhoIs



void cWhoIs::Process()
{
	if(mRequestPending)
	{
		// when the socket closes you have the whole message
		if(mSocket.ConnectionEstablished())
		{
			s32 bytesRcvd;	
			u8 buf[512];
			do
			{
				bytesRcvd = mSocket.Recv(buf, sizeof(buf));
				if(bytesRcvd > 0)
				{
					// the byte stream will expand as needed
					mByteStream.StreamBytes(buf, bytesRcvd);
				}
			}
			while(bytesRcvd > 0);
		}
		else
		{
			mByteStream.Trim();

			mRequestPending=false;
			if(mCb)
			{
				bool success = (mByteStream.Size() > 0);
				mCb(success, mByteStream.Data(), mpParam);
			}	
		}
	}
}// END Process



bool cWhoIs::Request(const char* serverDomainName, const char* query, u32 querySize, WhoIsRequestCompleteCb cb, void* param)
{
	bool ret;
	cIpAddr ip;
	
	ret = cDns::IpAddressFromDomainName(serverDomainName, 0, &ip);
	if(!ret)
	{
		return false;
	}

	Printf("server ip %s\n", ip.AsString());

	cSockAddr addr(ip, WHOIS_PORT);
	ret = mSocket.OpenAndConnect(addr, true);				// <<-- blocks for connection here
	if(!ret)
	{
		return false;
	}
	
	ret = mSocket.Send(query, querySize);
	if(!ret)
	{
		return false;
	}

	mByteStream.ClearAndResize();
	mCb = cb;
	mpParam = param;
	mRequestPending = true;
	return true;
}// END Request





} // namespace net