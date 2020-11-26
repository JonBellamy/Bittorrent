// Jon Bellamy 26-01-2008
// Common port numbers


#ifndef _PORTS_H
#define _PORTS_H


namespace net 
{

	enum
	{
		POP3_PORT = 110,
		POP3_SSL_PORT = 995,
		SMTP_PORT = 25,
		SMTP_SSL_PORT = 465,
		HTTP_PORT = 80,
		HTTPS_PORT = 443,
		NNTP_PORT = 119,
		NNTP_SSL_PORT = 563,
		SNTP_PORT = 123,
		WHOIS_PORT = 43,
		XMPP_PORT = 5222,
		XMPP_PORT_SSL = 5223,
	};

} // namespace net


#endif 