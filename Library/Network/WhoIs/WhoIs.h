#ifndef WHOIS_H
#define WHOIS_H


#include "TcpSocket_NDS.h"
#include "IpAddr_NDS.h"
#include "Network/NDS/Core/bytestream.h"

namespace net {


class cWhoIs
{
public:
	cWhoIs();
	virtual ~cWhoIs();

private:
	cWhoIs(const cWhoIs& rhs);
	const cWhoIs& operator= (const cWhoIs& rhs);

public:

	typedef void (*WhoIsRequestCompleteCb) (bool success, const u8* result, void* param);

	bool Request(const char* server, const char* query, u32 querySize, WhoIsRequestCompleteCb cb, void* param=NULL);
	void Process();

	enum
	{
		DEFAULT_BUFFER_SIZE = 512,
		WHOIS_PORT = 43					// TODO : Move this 
	};

private:
	cTcpSocket mSocket;
	cByteStream mByteStream;
	bool mRequestPending;

	WhoIsRequestCompleteCb mCb;
	void* mpParam;
};

} // namespace net

#endif // WHOIS_H