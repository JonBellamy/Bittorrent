// Simple Network Time Protocol (SNTP)
// RFC 1361

#ifndef SNTP_H
#define SNTP_H

#include <string>
#include "Network/UdpSocket.h"
#include "Network/IpAddr.h"
#include "Network/Ports.h"

namespace net {


class cSntp
{
public:
	cSntp();
	virtual ~cSntp();

private:
	cSntp(const cSntp& rhs);
	const cSntp& operator= (const cSntp& rhs);

public:

	
	typedef void (*SntpTimeSyncCompleteCb) (bool success, const u32 secondsSince1900, const std::string date, void* param);

	bool Request(const char* serverName, SntpTimeSyncCompleteCb completeCb, void* cbParam=NULL);
	void Process();

	bool IsProcessingRequest() const { return mRequestPending; }

private:

	bool IsLeapYear(u32 year);
	std::string FormatDateTime(u32 seconds);

	enum
	{
		SNTP_VERSION_NUMBER = 3,

		MODE_SYMMETRIC_ACTIVE = 1,
		MODE_SYMMETRIC_PASSIVE = 2,
		MODE_CLIENT = 3,
		MODE_SERVER = 4,
		MODE_BROADCAST = 5
	};


	typedef struct  
	{
		// reversed these 3 to make layout correct
		u8 mMode : 3;
		u8 mVersionNumber : 3;
		u8 mLeapIndicator : 2;
		
		
		u8 mStratum;
		u8 mPollInterval;
		u8 mPrecision;
		s32 mRootDelay;						// fixed point number
		u32 mRootDispersion;
		u32 mReferenceClockIdentifier;
		u64 mReferenceTimestamp;
		u64 mOriginateTimestamp;
		u64 mReceiveTimestamp;
		u64 mTransmitTimestamp;
		
		// optional 96 byte authenticator can follow
	}sSntpMessage;

	cUdpSocket mSocket;
	bool mRequestPending;

	SntpTimeSyncCompleteCb mCompleteCb;
	void* mCompleteCbParam;
};

} // namespace net

#endif // WHOIS_H