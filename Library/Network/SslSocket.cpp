// Jon Bellamy 21/03/2008
// SSL socket class using OpenSSL

#if USE_PCH
#include "stdafx.h"
#endif

#include "SslSocket.h"

#include <assert.h>

#include "openssl/rand.h"
#include "openssl/err.h"
#include "openssl/x509.h"
#include "openssl/rsa.h"
#include "openssl/evp.h"

#include "General/Rand.h"

#include "Network/bytestream.h"


// this should be defined in the project or even better yet, use a windows define
//#define USING_DOT_NET 0



using namespace std;


namespace net {



cSslSocket::cSslSocket(u32 sendBufferSize, u32 recvBufferSize, int keysize)
: cTcpSocket(INVALID_SOCKET, sendBufferSize, recvBufferSize)
, mHaveCertificate(false)
, mRsaKeySize(keysize)
, mpSslContext(NULL)
, mpSsl(NULL)
, mIncomingConnectionBlockMode(true)
, mConnectionSecured(false)
{
	// start ssl (one time only)
	static bool firstuse = true;
	if(firstuse)
	{
		// make sure winsock is open
		WinSock().Open();

		SslWrapper::SSL_load_error_strings();
		SslWrapper::SSL_library_init();
		
		// seed with time()
		time_t seed = time(NULL);
		srand(static_cast<u32>(seed));
       
		int tmp;		
		// Seed PRNG if needed
		while( SslWrapper::RAND_status() == 0 )
		{
			//PRNG may need lots of seed data
			tmp = rand();

			// add more random
			tmp += Rand32();

			SslWrapper::RAND_seed(&tmp, sizeof(int));
		}
		
		firstuse = false;
	}

	if(!CreateSslContext())
	{
		assert(0);
		return;
	}

	if(mpSsl == NULL)
	{
		mpSsl = SslWrapper::SSL_new(mpSslContext);
	}
	else
	{
		SslWrapper::SSL_clear(mpSsl);  //reuse old
	}
}// END cSslSocket



cSslSocket::~cSslSocket()
{
	if(mpSsl)
	{
		SslWrapper::SSL_free(mpSsl);
	}

	if(mpSslContext)
	{
		SslWrapper::SSL_CTX_free(mpSslContext);
	}
}// END ~cSslSocket



bool cSslSocket::Close()
{
	assert(mpSsl);
	//Send SSL shutdown signal to peer
	int ret = SslWrapper::SSL_shutdown(mpSsl);
	assert(ret != -1);
	return (cTcpSocket::Close() && ret != -1);
}// END Close



// secure open
bool cSslSocket::OpenAndConnect(const cSockAddr& addr, bool bBlocking)
{
	if(!mpSsl)
	{
		assert(0);
		return false;
	}

	// do the standard tcp connect (blocks for now)
	if(!cTcpSocket::OpenAndConnect(addr, true))
	{
		return false;
	}


	SecureConnection();

	// now we can use our blocking state
	SetBlockingState(bBlocking);

	Printf("SSL socket connected.\n");

	return true;
}// END OpenAndConnect



bool cSslSocket::SecureConnection()
{
	assert(!mConnectionSecured);

	// sets ssl to work in client mode, opposite of SSL_set_accept_state
	SslWrapper::SSL_set_connect_state(mpSsl);

	//get SSL to use our socket
	if(SslWrapper::SSL_set_fd(mpSsl, static_cast<int> (mSocket)) < 1)
	{
		return false;
	}

	s32 ret;
	// get SSL to handshake with server, the tcp 3 way handshake must have already taken place or this will fail
	ret = SslWrapper::SSL_connect(mpSsl);
	if(ret < 1)
	{
		return false;
	}

	// if we are not blocking for the connection phase then we had better delay the ssl setup phase thats about to happen
	assert(ConnectionEstablished());

	mConnectionSecured = true;

	return true;
}// END SecureConnection



bool cSslSocket::OpenAndListen(const cSockAddr& addr, bool bBlocking)
{
	mIncomingConnectionBlockMode = bBlocking;
	return cTcpSocket::OpenAndListen(addr, true);
}// END OpenAndListen



bool cSslSocket::ProcessIncomingConnection(cSslSocket& newConnection, cSockAddr& addrOut) 
{ 
	if(cTcpSocket::ProcessIncomingConnection(newConnection, addrOut))
	{
		// TODO : need to support unsecured incoming connections
		mConnectionSecured = true;

		// We have a new tcp connection, setup for ssl ...
		
		// Copy CTX object pointer
		newConnection.mpSslContext = mpSslContext;
		if(mpSslContext)
		{
			mpSslContext->references++;  // We don't want our destructor to delete ctx if still in use
		}

		// Does CTX have cert loaded?
		newConnection.mHaveCertificate = mHaveCertificate; 

		// Init SSL connection (server side)
		if(!newConnection.SslAccept())
		{
			newConnection.Close();
			return false;
		}

		newConnection.SetBlockingState(mIncomingConnectionBlockMode);

		Printf("SSL socket connected\n");
		return true;
	}

	return false; 
}// END ProcessIncomingConnection



s32 cSslSocket::Send(const void* buf, u32 bufSize)
{
	if(!mConnectionSecured)
	{
		return cTcpSocket::Send(buf, bufSize);
	}

	s32 ret;

	if(mpSsl == NULL)
	{
		return -1;
	}

	if ((ret = SslWrapper::SSL_write(mpSsl, buf, bufSize)) < 1)
	{
		return false;
	}

	if(ret < 0)
	{
		int err = SSL_get_error(mpSsl, ret);
		//Printf("err = %d\n", err);

		switch(err)
		{
		// The operation did not complete, the same TLS/SSL I/O function should be called again later
		case SSL_ERROR_WANT_WRITE:
			return 0;	
		default:
			return ret;
		}
	}

	return ret;
}// END Send



s32 cSslSocket::Recv(void* buf, u32 bufSize, bool bPeak) const
{
	if(!mConnectionSecured)
	{
		return cTcpSocket::Recv(buf, bufSize, bPeak);
	}

	s32 ret;

	if(mpSsl == NULL)
	{
		return -1;
	}

	if(bPeak)
	{
		ret = SslWrapper::SSL_peek(mpSsl, buf, bufSize);
	}
	else
	{
		ret = SslWrapper::SSL_read(mpSsl, buf, bufSize);
	}

	if(ret < 0)
	{
		int err = SslWrapper::SSL_get_error(mpSsl, ret);
		//Printf("err = %d\n", err);

		switch(err)
		{
		// The operation did not complete, the same TLS/SSL I/O function should be called again later
		case SSL_ERROR_WANT_READ:
			return 0;	
		default:
			return ret;
		}
	}

	return ret;
}// END Recv



bool cSslSocket::CreateSslContext()
{
	if(!mpSslContext)
	{
		//init new generic CTX object
		mpSslContext = SslWrapper::SSL_CTX_new(SslWrapper::SSLv23_method());
		
		if(!mpSslContext)
		{
			//handle_ERRerror(error, fatal, "cSslSocket::create_ctx() ");
			return false;
		}
		
		//SSL_CTX_set_options(ctx, SSL_OP_ALL);
		SslWrapper::SSL_CTX_set_mode(mpSslContext, SSL_MODE_AUTO_RETRY);
	}
	
	return true;
}// END CreateSslContext



bool cSslSocket::SslAccept()
{	
	if(!CreateSslContext())
	{
		return false;
	}
	
	if(mpSsl)
	{
		assert(0);
		// Shouldn't be possible...
		SslWrapper::SSL_free(mpSsl);
		mpSsl = NULL;
	}
	
	if(VerifyWeHaveACertificate() == false)
	{
		return false;
	}
	
	mpSsl = SslWrapper::SSL_new(mpSslContext);
	if(!mpSsl)
	{
		return false;
	}
	
	// sets ssl to work in server mode, opposite of SSL_set_connect_state
	SslWrapper::SSL_set_accept_state(mpSsl);
	
	//get SSL to use our socket
	if(SslWrapper::SSL_set_fd(mpSsl, static_cast<int> (mSocket)) < 1)
	{
		return false;
	}
	
	s32 ret;
	ret = SslWrapper::SSL_accept(mpSsl);
	//get SSL to handshake with client
	if(ret < 1)
	{
		return false;
	}
	
	return true;
}// END SslAccept



RSA* cSslSocket::GenerateRsaKey(int len, int exp) 
{
	return SslWrapper::RSA_generate_key(len,exp,NULL,NULL);
}// END GenerateRsaKey



EVP_PKEY* cSslSocket::GeneratePrivateKey(RSA *rsakey) 
{
	EVP_PKEY *pkey=NULL;

	if(!(pkey=SslWrapper::EVP_PKEY_new()))
	{
		return NULL;
	}

	if (!SslWrapper::EVP_PKEY_assign_RSA(pkey, rsakey))
	{
		SslWrapper::EVP_PKEY_free(pkey);
		return NULL;
	}

	return(pkey);
}// END GeneratePrivateKey



X509* cSslSocket::BuildCertificate(char *name, char *organization, char *country, EVP_PKEY *key) 
{
	if(!name)
	{
		return NULL;  // At least a name should be provided
	}

	// Create an X509_NAME structure to hold the distinguished name 
	X509_NAME* n = SslWrapper::X509_NAME_new();
	if(!n)
	{
		return NULL;
	}

	// Add fields
	if (!SslWrapper::X509_NAME_add_entry_by_NID(n, NID_commonName, MBSTRING_ASC, (unsigned char*)name, -1, -1, 0))
	{
		SslWrapper::X509_NAME_free(n);
		return NULL;
	}

	if(organization)
	{
		if (!SslWrapper::X509_NAME_add_entry_by_NID(n, NID_organizationName, MBSTRING_ASC, (unsigned char*)organization, -1, -1, 0))
		{
			SslWrapper::X509_NAME_free(n);
			return NULL;
		}
	}

	if(country)
	{
		if (!SslWrapper::X509_NAME_add_entry_by_NID(n, NID_countryName, MBSTRING_ASC, (unsigned char*)country, -1, -1, 0))
		{
			SslWrapper::X509_NAME_free(n);
			return NULL;
		}
	}


	X509 *c = SslWrapper::X509_new();
	if(!c)
	{
		SslWrapper::X509_NAME_free(n);
		return NULL;
	}

	// Set subject and issuer names to the X509_NAME we made
	SslWrapper::X509_set_issuer_name(c, n);
	SslWrapper::X509_set_subject_name(c, n);
	SslWrapper::X509_NAME_free(n);

	// Set serial number to zero
	SslWrapper::ASN1_INTEGER_set(X509_get_serialNumber(c), 0);

	// Set the valid/expiration times
	ASN1_UTCTIME *s = SslWrapper::ASN1_UTCTIME_new();
	if(!s)
	{
		SslWrapper::X509_free(c);
		return NULL;
	}

	SslWrapper::X509_gmtime_adj(s, -60*60*24);
	SslWrapper::X509_set_notBefore(c, s);
	SslWrapper::X509_gmtime_adj(s, 60*60*24*364);
	SslWrapper::X509_set_notAfter(c, s);

	SslWrapper::ASN1_UTCTIME_free(s);

	// Set the public key
	SslWrapper::X509_set_pubkey(c, key);

	// Self-sign it
	SslWrapper::X509_sign(c, key, EVP_sha1());

	return(c);
}// END BuildCertificate



// Create temp cert if needed
bool cSslSocket::VerifyWeHaveACertificate()
{
	if(mHaveCertificate)
	{
		return true;  // No need to create a new temp cert
	}
	
/*
	// JonB
	assert(0);
	return false;

	// the code below was commented out previously for win forms / dll issues, it has to stay now so fix it !!!
*/
	
	// JonB TODO : make these a member so we have certificates per connection?
	static bool created_session_data = false;
	static RSA *rsa_key = NULL;
	static EVP_PKEY *evp_pkey = NULL;
	static X509 *cert = NULL;
	
	// Create a session certificate (global for all instances of this class) if no other certificate was provided
	if(!created_session_data)
	{	
		if(!rsa_key)
		{
			if(!(rsa_key = GenerateRsaKey(mRsaKeySize)))
			{
				return false;	
			}
		}
		
		if(!evp_pkey)
		{
			if( !(evp_pkey = GeneratePrivateKey(rsa_key)) )
			{
				return false;
			}
		}
		
		if(!(cert = BuildCertificate("session certificate", NULL, NULL, evp_pkey)))
		{
			return false;
		}
		
		created_session_data = true;
	}
	
	// Use our session certificate
	SslWrapper::SSL_CTX_use_RSAPrivateKey(mpSslContext, rsa_key);
	SslWrapper::SSL_CTX_use_certificate(mpSslContext, cert);

	mHaveCertificate = true;
	
	return true;
}// END VerifyWeHaveACertificate








//////////////////////////////////////////////////////////////////////////
// for now unreferenced 


// Load a certificate. A socket used on the server side needs to have a certificate, but a temporary RSA session certificate will be created if you don't load one yourself. 
// Every server side socket in your application that doesn't load a certificate will have the same session certificate (a global certificate will be created the first time one is needed). 
bool cSslSocket::LoadCertificate(const char *cert_file, const char *private_key_file, SslWrapper::pem_password_cb* passwd_cb)
{
	if(!cert_file || !private_key_file)
	{
		return false;
	}

	if(!CreateSslContext())
	{
		return false;
	}

	mHaveCertificate = false;

	// Load CERT PEM file
	if(!SslWrapper::SSL_CTX_use_certificate_chain_file(mpSslContext, cert_file))
	{
		return false;	
	}

	// Load private key PEM file
	if(passwd_cb)
	{
		SslWrapper::SSL_CTX_set_default_passwd_cb(mpSslContext, passwd_cb);
	}

	/*
	// Give passwd callback if any
	if( userdata ) // <<-- cb param is all this is
	{
		SSL_CTX_set_default_passwd_cb_userdata(ctx, (void *)userdata);
	}
	*/

	for(int i=0; i<3; i++)
	{
		if( SslWrapper::SSL_CTX_use_PrivateKey_file(mpSslContext, private_key_file, SSL_FILETYPE_PEM) )
		{
			break;
		}

		if(ERR_GET_REASON(SslWrapper::ERR_peek_error())==EVP_R_BAD_DECRYPT)
		{
			// Give the user two tries
			if(i<2)
			{
				SslWrapper::ERR_get_error(); //remove from stack
				continue;
			}

			return false;
		}

		return false;
	}

	// Check private key
	if(!SslWrapper::SSL_CTX_check_private_key(mpSslContext))
	{
		return false;	
	}

	mHaveCertificate = true;

	return true;
}// END LoadCertificate



int cSslSocket::verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
	// We don't care. Continue with handshake
	return 1; /* Accept connection */
}// END verify_callback



// Should the peer certificate be verified? The arguments specify the locations of trusted CA certificates used in the verification. Either ca_file or ca_dir can be set to NULL but not both. 
// See man SSL_CTX_load_verify_locations(3)  for format information. Should be called before accept() or connect() if used and the verification result is then available by 
// calling get_verify_result() on the connected socket (the new socket from accept() on the server side, the same socket on the client side). Returns true on success. 
// Possible SSL error types: badFile. Possible normal error types: fatal.
bool cSslSocket::UseVerification(const char *ca_file, const char *ca_dir)
{
	if(!ca_file && !ca_dir)
	{
		// We must have at least one set
		return false;
	}

	if( !CreateSslContext() )
	{
		return false;
	}

	if( ca_file )
	{
		if( !SslWrapper::SSL_CTX_load_verify_locations(mpSslContext, ca_file, NULL) )
		{
			//handle_ERRerror(error, badFile, "cSslSocket::use_verification() on file " + string(ca_file) + " ");
			return false;
		}
	}

	if( ca_dir )
	{
		if( !SslWrapper::SSL_CTX_load_verify_locations(mpSslContext, NULL, ca_dir) )
		{
			//handle_ERRerror(error, badFile, "cSslSocket::use_verification() on dir " + string(ca_dir) + " ");
			return false;
		}
	}

	SslWrapper::SSL_CTX_set_verify(mpSslContext, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, verify_callback);

	//no_error(error);
	return true;
}// END UseVerification



// Load Diffie-Hellman parameters from file. These are used to generate a DH key exchange. See man SSL_CTX_set_tmp_dh_callback(3) and www.skip-vpn.org/spec/numbers.html for more 
// information. Should be called before accept() or connect() if used. Returns true on success. Possible SSL error types: badFile (only if the file exist but is invalid). 
// Possible normal error types: fatal (couldn't open file).
bool cSslSocket::use_DHfile(const char* dh_file)
{
	if(!dh_file)
	{
		return false;
	}

	if(!CreateSslContext())
	{
		return false;
	}

	// Set up DH stuff
	DH *dh;
	FILE *paramfile;

	paramfile = fopen(dh_file, "r");
	if(paramfile)
	{
		dh = SslWrapper::PEM_read_DHparams(paramfile, NULL, NULL, NULL);
		fclose(paramfile);

		if(!dh)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	SslWrapper::SSL_CTX_set_tmp_dh(mpSslContext, dh);

	SslWrapper::DH_free(dh);

	return true;
}// END use_DHfile


// for now unreferenced 
//////////////////////////////////////////////////////////////////////////




} // namespace net

