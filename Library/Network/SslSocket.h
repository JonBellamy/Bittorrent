// Jon Bellamy 21/03/2008
// SSL socket class using OpenSSL


#ifndef SSL_SOCK_H
#define SSL_SOCK_H


#include "Network/TcpSocket.h"
#include <string>
#include <assert.h>

#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/pem.h>


#if USING_DOT_NET
#include "Network/OpenSslWrapper.h"
#endif

#if !USING_DOT_NET
#define SslWrapper 
#endif


namespace net {



class cSslSocket : public cTcpSocket
{
public:
	cSslSocket(u32 sendBufferSize=DEFAULT_SOCKET_SEND_BUFFER_SIZE, u32 recvBufferSize=DEFAULT_SOCKET_RECV_BUFFER_SIZE, int keysize=RSA_KEYSIZE);
	virtual ~cSslSocket();

private:	
	void operator= (SOCKET s) {}
	cSslSocket(const cSslSocket& rhs);
	cSslSocket(SOCKET s);


public:
	// secure
	bool OpenAndConnect(const cSockAddr& addr, bool bBlocking=false);
	// not secure
	bool OpenAndConnectUnsecured(const cSockAddr& addr, bool bBlocking=false) { return cTcpSocket::OpenAndConnect(addr, bBlocking); }

	bool SecureConnection();
	
	bool OpenAndListen(const cSockAddr& addr, bool bBlocking=false);
	bool ProcessIncomingConnection(cSslSocket& newConnection, cSockAddr& addrOut);

	bool Close();
	
	s32 Send(const void* buf, u32 bufSize);
	s32 Recv(void* buf, u32 bufSize, bool peak=false) const;
	s32 Recv(cByteStream* pBuffer, u32 maxRecv, bool peak=false) const { return cTcpSocket::Recv(pBuffer, maxRecv, peak); }

private:

	enum
	{
		// Default values for RSA key generation
		RSA_KEYSIZE = 512,
		RSA_KEYEXP = RSA_F4			// 65537
	};


	// used when processing incoming connections
	bool SslAccept(); 

	// Create new CTX if none is available
	bool CreateSslContext();


	RSA* GenerateRsaKey(int len, int exp=RSA_KEYEXP);
	EVP_PKEY* GeneratePrivateKey(RSA *rsakey);
	X509* cSslSocket::BuildCertificate(char *name, char *organization, char *country, EVP_PKEY *key);
	
	// Will create a temporary certificate if no other is loaded
	bool VerifyWeHaveACertificate();

	

	
	//////////////////////////////////////////////////////////////////////////
	// for now unreferenced 

	bool LoadCertificate(const char *cert_file, const char *private_key_file, SslWrapper::pem_password_cb* passwd_cb);

	static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx);

	// Should the peer certificate be verified? The arguments specify the locations of trusted CA certificates used in the verification. 
	// Either ca_file or ca_dir can be set to NULL but not both. See man SSL_CTX_load_verify_locations(3)  for format information. 
	// Should be called before accept() or connect() if used and the verification result is then available by calling get_verify_result() on 
	// the connected socket (the new socket from accept() on the server side, the same socket on the client side). Returns true on success. 
	// Possible SSL error types: badFile. Possible normal error types: fatal.
	bool UseVerification(const char *ca_file, const char *ca_dir);

	// Load Diffie-Hellman parameters from file. These are used to generate a DH key exchange. See man SSL_CTX_set_tmp_dh_callback(3) and www.skip-vpn.org/spec/numbers.html for more information. 
	// Should be called before accept() or connect() if used. Returns true on success. Possible SSL error types: badFile (only if the file exist but is invalid).
	// Possible normal error types: fatal (couldn't open file).
	bool use_DHfile(const char* dh_file);


	// Get peer certificate verification result
	// Should be called after connect() or accept() where the verification is done.
	// On the server side (i.e accept()) this should be done on the new class returned
	// by accept() and NOT on the listener class!
	typedef enum 
	{
		noCert, 
		CertOk, 
		noIssCert, 
		CertExpired, 
		CertSelfSign, 
		CertError
	}verify_result;
	struct verify_info
	{
		verify_result result;
		std::string error;
	};
	verify_info get_verify_result(void);


	// Get information about peer certificate
	// Should be called after connect() or accept() when using verification
	struct peerCert_info
	{
		// Issuer name
		std::string commonName;             // CN
		std::string countryName;            // C
		std::string localityName;           // L
		std::string stateOrProvinceName;    // ST
		std::string organizationName;       // O
		std::string organizationalUnitName; // OU
		std::string title;                  // T
		std::string initials;               // I
		std::string givenName;              // G
		std::string surname;                // S
		std::string description;            // D
		std::string uniqueIdentifier;       // UID
		std::string emailAddress;           // Email

		// Expire dates
		std::string notBefore;
		std::string notAfter;

		// Misc. data
		long serialNumber;
		long version;
		std::string sgnAlgorithm;
		std::string keyAlgorithm;
		int keySize;
	};

	// for now unreferenced 
	//////////////////////////////////////////////////////////////////////////

	


	//////////////////////////////////////////////////////////////////////////
	// SSL data

	// A SSL_CTX object is created as a framework to establish TLS/SSL enabled connections (see SSL_CTX_new(3)). 
	// Various options regarding certificates, algorithms etc can be set in this object. 
	SSL_CTX* mpSslContext;	

	// This is the main SSL/TLS structure which is created by a server or client per established connection. 
	// This actually is the core structure in the SSL API. Under run-time the application usually deals with this structure which has links to mostly all other structures. 
	SSL* mpSsl;	

	bool mHaveCertificate;					// Indicate CERT loaded or created
	int mRsaKeySize;						// keysize argument from constructor

	bool mIncomingConnectionBlockMode;

	bool mConnectionSecured;
};


} // namespace net


#endif // SSL_SOCK_H


