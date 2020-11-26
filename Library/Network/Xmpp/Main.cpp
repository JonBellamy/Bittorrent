#if 0
#include "Network/Dns.h"
#include "Network/NetworkAdaptorList.h"
#include "Network/Ports.h"
#include "Network/TcpSocket.h"
#include "Network/SslSocket.h"
#include "Network/WinSock.h"
#include "Network/SockAddr.h"
#include "Network/bytestream.h"
#include "Network/Base64.h"


using namespace net; 


cSslSocket gSslSocket;
u8 recvBuffer[1024*32];


void RecvResponse()
{
	s32 bytesRecvd;
	bytesRecvd=0;
	memset(recvBuffer, 0, sizeof(recvBuffer));
	bytesRecvd = gSslSocket.Recv(recvBuffer, sizeof(recvBuffer), false);
	assert(bytesRecvd >= 0);
	printf("\n%s\n", reinterpret_cast<char*>(recvBuffer));
}// END RecvResponse



int main()
{
	WinSock().Open();

	cIpAddr ip;
	cDns::IpAddressFromDomainName("jabberd.eu", 0, &ip);
	cSockAddr sockAddr(ip, XMPP_PORT);
	u32 bytesSent;
	bool bRet;
	
	bRet = gSslSocket.OpenAndConnectUnsecured(sockAddr, true);
	assert(bRet);

	// new stream
	char streamStr[] = "<stream:stream xmlns=\"jabber:client\" xmlns:stream=\"http://etherx.jabber.org/streams\" id=\"2843556837\" to=\"jabberd.eu\" version=\"1.0\" xml:lang=\"en\">\r\n";
	printf("%s\n", streamStr);
	bytesSent = gSslSocket.Send(streamStr, sizeof(streamStr)-1);

	RecvResponse();
	assert(gSslSocket.ConnectionEstablished());

	// Start TLS
	char startTls[] = "<starttls xmlns=\"urn:ietf:params:xml:ns:xmpp-tls\"/>";
	printf("\n%s\n", startTls);
	bytesSent = gSslSocket.Send(startTls, sizeof(startTls)-1);

	RecvResponse();
	assert(gSslSocket.ConnectionEstablished());


	// Fire up OpenSSL & secure the socket 
	bRet = gSslSocket.SecureConnection();
	assert(bRet);
	

	// new stream
	printf("\n%s\n", streamStr);
	bytesSent = gSslSocket.Send(streamStr, sizeof(streamStr)-1);
	RecvResponse();


	// plain SASL
	cByteStream saslByteStream(1024);
	u8 nullByte = NULL;
	char username[] = "jonb_rs2";
	char password[] = "4333d9211b";
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (&nullByte), 1);
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (username), sizeof(username)-1);
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (&nullByte), 1);
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (password), sizeof(password)-1);

	// Base64 encode the auth details, then send
	char b64Auth[1024];
	memset(b64Auth, 0, sizeof(b64Auth));
	s32 b64AuthSize = EncodeBase64(reinterpret_cast<const u8*> (saslByteStream.Data()), saslByteStream.Size(), reinterpret_cast<u8*> (b64Auth), sizeof(b64Auth));

	char authXml1[] = "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">\r\n";
	char authXml2[] = "</auth>\r\n";
	printf("\n\n%s%s\n%s", authXml1, b64Auth, authXml2);

	bytesSent = gSslSocket.Send(authXml1, sizeof(authXml1)-1);
	bytesSent = gSslSocket.Send(b64Auth, b64AuthSize);
	bytesSent = gSslSocket.Send(authXml2, sizeof(authXml2)-1);
	
	RecvResponse();


	// new stream
	printf("\n%s\n", streamStr);
	bytesSent = gSslSocket.Send(streamStr, sizeof(streamStr)-1);
	RecvResponse();

	// bind resource
	char resourceBind[] = "<iq type=\"set\" id=\"myid1\">\r\n<bind xmlns=\"urn:ietf:params:xml:ns:xmpp-bind\">\r\n<resource>JonsTestRes</resource>\r\n</bind>\r\n</iq>\r\n";
	printf("\n%s\n", resourceBind);
	bytesSent = gSslSocket.Send(resourceBind, sizeof(resourceBind)-1);
	RecvResponse();

	// session
	char session[] = "<iq type=\"set\" id=\"myid2\">\r\n<session xmlns=\"urn:ietf:params:xml:ns:xmpp-session\" />\r\n</iq>\r\n";
	printf("\n%s\n", session);
	bytesSent = gSslSocket.Send(session, sizeof(session)-1);
	RecvResponse();

	// get roster
	char getRosterQuery[] = "<iq type=\"get\" id=\"myid3\">\r\n<query xmlns=\"jabber:iq:roster\" />\r\n</iq>\r\n";
	printf("\n%s\n", getRosterQuery);
	bytesSent = gSslSocket.Send(getRosterQuery, sizeof(getRosterQuery)-1);
	RecvResponse();


	// set presence - online
	char presenceOnline[] = "<presence from=\"jonb_rs2@jabberd.eu/JonsTestRes\" to=\"jonb_rs1@jabberd.eu\">\r\n<show>chat</show>\r\n<status>online from test project</status>\r\n</presence>\r\n";
	printf("\n%s\n", presenceOnline);
	bytesSent = gSslSocket.Send(presenceOnline, sizeof(presenceOnline)-1);
	RecvResponse();

	// send message
	char chatMessage[] = "<message type=\"chat\" to=\"jonb_rs1@jabberd.eu\" id=\"myid4\">\r\n<body>hello world</body>\r\n<active xmlns=\"http://jabber.org/protocol/chatstates\" />\r\n</message>\r\n";
	printf("\n%s\n", chatMessage);
	bytesSent = gSslSocket.Send(chatMessage, sizeof(chatMessage)-1);
	RecvResponse();


	char streamEnd[] = "</stream:stream>";
	gSslSocket.Send(streamEnd, sizeof(streamEnd)-1);

	gSslSocket.Close();

	return 0;
}// End main


#endif