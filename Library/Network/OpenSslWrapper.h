// defines all the functions we will use in the OpenSSL dll's
// If you need to use an ssl function that isn't in this list, add it and call through the wrapper. 
// You cannot simply call the function or the silly .net linker will cry like the whinny little girl it is.



#ifndef __SSL_WRAP_H
#define __SSL_WRAP_H


using namespace System;
using namespace System::Windows::Forms;
using namespace System::Runtime::InteropServices;

#include <openssl/ssl.h>

//namespace net 
//{

public ref class SslWrapper
{
public:

	//////////////////////////////////////////////////////////////////////////
	// SSLeay32.dll imports

	[DllImport("SSleay32.dll", EntryPoint="SSL_library_init")]
	static int SSL_library_init(void);

	[DllImport("SSleay32.dll", EntryPoint="SSL_load_error_strings")]
	static void SSL_load_error_strings(void );

	[DllImport("SSleay32.dll", EntryPoint="SSLv3_client_method")]
	static SSL_METHOD* SSLv3_client_method(void);

	[DllImport("SSleay32.dll", EntryPoint="SSLv2_client_method")]
	static SSL_METHOD* SSLv2_client_method(void);

	[DllImport("SSleay32.dll", EntryPoint="SSLv23_client_method")]
	static SSL_METHOD* SSLv23_client_method(void);

	[DllImport("SSleay32.dll", EntryPoint="TLSv1_client_method")]
	static SSL_METHOD* TLSv1_client_method(void);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_new")]
	static SSL_CTX* SSL_CTX_new(SSL_METHOD *meth);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_set_cipher_list")]
	static int	SSL_CTX_set_cipher_list(SSL_CTX *,const char *str);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_use_certificate")]
	static int SSL_CTX_use_certificate(SSL_CTX *ctx, X509 *x);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_use_certificate_file")]
	static int	SSL_CTX_use_certificate_file(SSL_CTX *ctx, const char *file, int type);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_use_PrivateKey_file")]
	static int	SSL_CTX_use_PrivateKey_file(SSL_CTX *ctx, const char *file, int type);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_check_private_key")]
	static int SSL_CTX_check_private_key(const SSL_CTX *ctx);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_set_default_verify_paths")]
	static int SSL_CTX_set_default_verify_paths(SSL_CTX *ctx);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_load_verify_locations")]
	static int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile, const char *CApath);

	[DllImport("SSleay32.dll", EntryPoint="SSL_new")]
	static SSL*	SSL_new(SSL_CTX *ctx);

	[DllImport("SSleay32.dll", EntryPoint="SSL_set_connect_state")]
	static void SSL_set_connect_state(SSL *s);

	[DllImport("SSleay32.dll", EntryPoint="SSL_set_bio")]
	static void	SSL_set_bio(SSL *s, BIO *rbio,BIO *wbio);

	[DllImport("SSleay32.dll", EntryPoint="SSL_set_fd")]
	static int	SSL_set_fd(SSL *s, int fd);

	[DllImport("SSleay32.dll", EntryPoint="SSL_connect")]
	static int 	SSL_connect(SSL *ssl);

	[DllImport("SSleay32.dll", EntryPoint="SSL_read")]
	static int 	SSL_read(SSL *ssl,void *buf,int num);
	
	[DllImport("SSleay32.dll", EntryPoint="SSL_write")]
	static int 	SSL_write(SSL *ssl,const void *buf,int num);

	[DllImport("SSleay32.dll", EntryPoint="SSL_peek")]
	static int 	SSL_peek(SSL *ssl,void *buf,int num);

	[DllImport("SSleay32.dll", EntryPoint="SSL_accept")]
	static int 	SSL_accept(SSL *ssl);

	[DllImport("SSleay32.dll", EntryPoint="SSL_clear")]
	static int	SSL_clear(SSL *s);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_set_default_passwd_cb")]
	static void SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx, pem_password_cb *cb);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_set_default_passwd_cb_userdata")]
	static void SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX *ctx, void *u);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_free")]
	static void	SSL_CTX_free(SSL_CTX *);

	[DllImport("SSleay32.dll", EntryPoint="SSL_shutdown")]
	static int SSL_shutdown(SSL *s);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_flush_sessions")]
	static void	SSL_CTX_flush_sessions(SSL_CTX *ctx,long tm);

	[DllImport("SSleay32.dll", EntryPoint="SSL_free")]
	static void	SSL_free(SSL *ssl);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_use_certificate_chain_file")]
	static int	SSL_CTX_use_certificate_chain_file(SSL_CTX *ctx, const char *file);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_set_tmp_dh_callback")]
	static void SSL_CTX_set_tmp_dh_callback(SSL_CTX *ctx, DH *(*dh)(SSL *ssl,int is_export, int keylength));

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_set_verify")]
	static void SSL_CTX_set_verify(SSL_CTX *ctx,int mode, int (*callback)(int, X509_STORE_CTX *));

	[DllImport("SSleay32.dll", EntryPoint="SSL_get_peer_certificate")]
	static X509* SSL_get_peer_certificate(const SSL *s);

	[DllImport("SSleay32.dll", EntryPoint="SSL_get_verify_result")]
	static long SSL_get_verify_result(const SSL *ssl);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_ctrl")]
	static long	SSL_CTX_ctrl(SSL_CTX *ctx,int cmd, long larg, void *parg);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_use_RSAPrivateKey")]
	static int SSL_CTX_use_RSAPrivateKey(SSL_CTX *ctx, RSA *rsa);

	[DllImport("SSleay32.dll", EntryPoint="SSL_CTX_use_RSAPrivateKey_file")]
	static int	SSL_CTX_use_RSAPrivateKey_file(SSL_CTX *ctx, const char *file, int type);

	[DllImport("SSleay32.dll", EntryPoint="SSL_set_accept_state")]
	static void SSL_set_accept_state(SSL *s);

	[DllImport("SSleay32.dll", EntryPoint="SSLv23_method")]
	static SSL_METHOD* SSLv23_method(void);

	[DllImport("SSleay32.dll", EntryPoint="RSA_generate_key")]
	static RSA*	RSA_generate_key(int bits, unsigned long e,void(*callback)(int,int,void *),void *cb_arg);


	//////////////////////////////////////////////////////////////////////////
	// libeay32.dll imports

	[DllImport("libeay32.dll", EntryPoint="BIO_new")]
	static BIO*	BIO_new(BIO_METHOD *type);

	[DllImport("libeay32.dll", EntryPoint="BIO_s_connect")]
	static BIO_METHOD *BIO_s_connect(void);

	[DllImport("libeay32.dll", EntryPoint="BIO_ctrl")]
	static long	BIO_ctrl(BIO *bp,int cmd,long larg,void *parg);

	[DllImport("libeay32.dll", EntryPoint="BIO_s_mem")]
	static BIO_METHOD *BIO_s_mem(void);

	[DllImport("libeay32.dll", EntryPoint="BIO_read")]
	static int	BIO_read(BIO *b, void *data, int len);

	[DllImport("libeay32.dll", EntryPoint="BIO_free")]
	static int	BIO_free(BIO *a);

	[DllImport("libeay32.dll", EntryPoint="X509_set_subject_name")]
	static int X509_set_subject_name(X509 *x, X509_NAME *name);

	[DllImport("libeay32.dll", EntryPoint="X509_set_issuer_name")]
	static int X509_set_issuer_name(X509 *x, X509_NAME *name);

	[DllImport("libeay32.dll", EntryPoint="ASN1_INTEGER_set")]
	static int ASN1_INTEGER_set(ASN1_INTEGER *a, long v);

	[DllImport("libeay32.dll", EntryPoint="PEM_ASN1_write_bio")]
	static int	PEM_ASN1_write_bio(i2d_of_void *i2d,const char *name,BIO *bp,char *x,const EVP_CIPHER *enc,unsigned char *kstr,int klen,pem_password_cb *cb, void *u);

	[DllImport("libeay32.dll", EntryPoint="EVP_PKEY_size")]
	static int EVP_PKEY_size(EVP_PKEY *pkey);

	[DllImport("libeay32.dll", EntryPoint="X509_get_pubkey")]
	static EVP_PKEY*	X509_get_pubkey(X509 *x);

	[DllImport("libeay32.dll", EntryPoint="OBJ_nid2sn")]
	static const char*	OBJ_nid2sn(int n);

	[DllImport("libeay32.dll", EntryPoint="OBJ_obj2nid")]
	static int	OBJ_obj2nid(const ASN1_OBJECT *o);

	[DllImport("libeay32.dll", EntryPoint="ASN1_INTEGER_get")]
	static long ASN1_INTEGER_get(ASN1_INTEGER *a);

	[DllImport("libeay32.dll", EntryPoint="ASN1_TIME_print")]
	static int ASN1_TIME_print(BIO *fp,ASN1_TIME *a);

	[DllImport("libeay32.dll", EntryPoint="X509_get_serialNumber")]
	static ASN1_INTEGER* X509_get_serialNumber(X509 *x);

	[DllImport("libeay32.dll", EntryPoint="DH_free")]
	static void	DH_free(DH *dh);

	[DllImport("libeay32.dll", EntryPoint="ERR_get_error")]
	static unsigned long ERR_get_error(void);

	[DllImport("libeay32.dll", EntryPoint="ERR_peek_error")]
	static unsigned long ERR_peek_error(void);
	
	[DllImport("libeay32.dll", EntryPoint="RAND_seed")]
	static void RAND_seed(const void *buf,int num);

	[DllImport("libeay32.dll", EntryPoint="RAND_status")]
	static int RAND_status(void);

	[DllImport("libeay32.dll", EntryPoint="EVP_PKEY_free")]
	static void EVP_PKEY_free(EVP_PKEY *pkey);

	[DllImport("libeay32.dll", EntryPoint="EVP_PKEY_assign")]
	static int EVP_PKEY_assign(EVP_PKEY *pkey,int type,char *key);

	[DllImport("libeay32.dll", EntryPoint="EVP_PKEY_new")]
	static EVP_PKEY*	EVP_PKEY_new(void);

	[DllImport("libeay32.dll", EntryPoint="BIO_push")]
	static BIO *	BIO_push(BIO *b,BIO *append);

	[DllImport("libeay32.dll", EntryPoint="BIO_write")]
	static int	BIO_write(BIO *b, const void *data, int len);

	[DllImport("libeay32.dll", EntryPoint="BIO_free_all")]	
	static void	BIO_free_all(BIO *a);

	[DllImport("libeay32.dll", EntryPoint="BIO_new_mem_buf")]	
	static BIO *BIO_new_mem_buf(void *buf, int len);

	[DllImport("libeay32.dll", EntryPoint="BIO_f_base64")]	
	static BIO_METHOD *BIO_f_base64(void);
}; 






//} // namespace net


#endif  // __SSL_WRAP_H