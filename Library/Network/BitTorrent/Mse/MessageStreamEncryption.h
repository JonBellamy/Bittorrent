// Jon Bellamy 12/04/2010
// http://wiki.vuze.com/w/Message_Stream_Encryption
// Encrypted Bittorrent socket. This class will operate as either PeerA or PeerB.
// PeerA initiates the connection, PeerB listens for connections


#ifndef MSENC_H
#define MSENC_H

#if USE_PCH
#include "stdafx.h"
#endif


#include "OpenSSL/bn.h"
#include "OpenSSL/rc4.h"

#include "Network/TcpSocket.h"



class cEncryptedTorrentConnection : public net::cTcpSocket
{
public:

	// Static constructor & destructor (called manually)
	static void InitCrypto();
	static void DeInitCrypto();

	cEncryptedTorrentConnection(const cEncryptedTorrentConnection& rhs);

	// Called by PeerA when creating an outgoing connection, they have the info hash at that point.
	cEncryptedTorrentConnection(const u8* infohash, u32 sendBufferSize=DEFAULT_SOCKET_SEND_BUFFER_SIZE, u32 recvBufferSize=DEFAULT_SOCKET_RECV_BUFFER_SIZE);
	// Called by PeerB to create a new socket for an incoming connection, they do NOT know which info hash (ie torrent) this 
	// connection is for yet, we will guess it from data in the forthcoming handshake.
	cEncryptedTorrentConnection(u32 sendBufferSize=DEFAULT_SOCKET_SEND_BUFFER_SIZE, u32 recvBufferSize=DEFAULT_SOCKET_RECV_BUFFER_SIZE);
	~cEncryptedTorrentConnection();

	const cEncryptedTorrentConnection& operator= (const cEncryptedTorrentConnection& rhs);


private:
	//cEncryptedTorrentConnection();
	bool operator==(const cEncryptedTorrentConnection& rhs);


public:
	bool OpenAndConnect(const net::cSockAddr& addr, bool bBlocking=false);
	bool Close();

	s32 Recv(void* pBuffer, u32 bufSize, bool peak=false) const;
	s32 Recv(cByteStream* pBuffer, u32 maxRecv, bool peak=false) const { return cTcpSocket::Recv(pBuffer, maxRecv, peak); }
	s32 Send(const void* pPacket, u32 packetSize);

	bool ConnectionEstablished() const;
	u32 BytesPendingOnInputBuffer() const;

	void Process();

	enum
	{
		CRYPTO_MODE_PLAIN_TEXT = 0x00000001, 
		CRYPTO_MODE_RC4 = 0x00000002
	};

	typedef enum
	{
		NOT_CONNECTED=0,
		
		// PeerA States
		CONNECTING,
		SENT_YA,
		GOT_YB,
		FOUND_VC,
		WAIT_FOR_PAD_D,
		CONNECTED_PEERA,


		// PeerB States
		WAITING_FOR_YA,
		WAITING_FOR_REQ1,
		FOUND_REQ1,
		FOUND_INFO_HASH,
		WAIT_FOR_PAD_C,
		WAIT_FOR_IA,
		CONNECTED_PEERB,		
	}eState;

	eState State() const { return mState; }
	bool WasIncomingConnection() const { return mWasIncomingConnection; }
	bool IsEncryptedConnection() const { return (ConnectionEstablished() && crypto_select == CRYPTO_MODE_RC4); }

private:

	//////////////////////////////////////////////////////////////////////////
	// Private member functions

	void Init();
	void GeneratePublicPrivateKey(BIGNUM* priv, BIGNUM* pub);
	void CalcEncryptionKey(bool a, const BIGNUM& s, const u8* skey, u8* keyOut);
	void InitEncryptionKeys(bool amPeerA);

	void SendPublicKeyPlusPadding(const BIGNUM& publicKey);

	// PeerA States
	void HandleYb();
	void FindVC();
	void HandleCryptoSelect();
	void HandlePadD();

	// PeerB States
	void HandleYa();
	void FindReq1();
	void CalculateSKey();
	void ProcessVC();
	void HandlePadC();
	void HandleIA();



	//////////////////////////////////////////////////////////////////////////
	// Private member data

	eState mState;
	bool mWasIncomingConnection;
	bool mMseHandshakeComplete;

	// used to encrypt data before its sent (so we don't destroy the original)
	cByteStream mCryptoSendBuffer;
	mutable cByteStream mCryptoRecvBuffer;

	
	


	// Both sides use the following ...
	// P & G are static as they are essentially const
	static BIGNUM* P;			// Safe prime, known by both sides before connection
	static BIGNUM* G;			// 0x02
	BIGNUM S;					// Shared secret, worked out after first data exchange
	u8 SKEY[20];				// Secret key -> info hash


	// PeerA: Connection initiator 
	BIGNUM Xa;					// PeerA private key (PeerB will never know this)
	BIGNUM Ya;					// PeerA public key (sent to PeerB)
	u32 vc_off;
	u32 crypto_select;
	u16 pad_D_len;
	u32 end_of_crypto_handshake;


	// PeerB: Incoming connection
	BIGNUM Xb;					// PeerB private key (PeerA will never know this)
	BIGNUM Yb;					// PeerB public key (sent to PeerA)
	u32 crypto_provide;
	u32 req1_off;
	u16 pad_C_len;
	u16 ia_len;


	// encryption / decryption keys
	u8 encryptionKey[20];
	u8 decryptionKey[20];
	mutable RC4_KEY encKey;
	mutable RC4_KEY decKey;

	static BN_CTX* gBnContext;
};





#endif // MSENC_H
