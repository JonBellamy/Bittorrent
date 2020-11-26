// Jon Bellamy 12/04/2010
// http://wiki.vuze.com/w/Message_Stream_Encryption
// Encrypted Bittorrent socket. This class will operate as either PeerA or PeerB.
// PeerA initiates the connection, PeerB listens for connections


#include "MessageStreamEncryption.h"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "OpenSSL/crypto.h"
#include "OpenSSL/sha.h"

#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/BitTorrent/BitTorrentMessages.h"
#include "File/file.h"
#include "General/Rand.h"

using namespace net;



BN_CTX* cEncryptedTorrentConnection::gBnContext = NULL;
BIGNUM* cEncryptedTorrentConnection::P = NULL;
BIGNUM* cEncryptedTorrentConnection::G = NULL;



// Static constructor & destructor (called manually)
void cEncryptedTorrentConnection::InitCrypto()
{
	gBnContext = BN_CTX_new();

	// Prime P is a 768 bit safe prime
	int ret = BN_hex2bn(&P, "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245E485B576625E7EC6F44C42E9A63A36210000000000090563");
	assert(P!=NULL && ret != 0 && ret == 192);
	u32 numBitsP = BN_num_bits(P);
	assert(numBitsP == 768);

	// Generator G is "2"
	G=NULL;
	ret = BN_hex2bn(&G,"02");
	assert(G!=NULL && ret != 0);
}// END InitCrypto



void cEncryptedTorrentConnection::DeInitCrypto()
{
	BN_free(P);
	BN_free(G);
	BN_CTX_free(gBnContext);
}// END DeInitCrypto




cEncryptedTorrentConnection::cEncryptedTorrentConnection(const cEncryptedTorrentConnection& rhs)
{
	Init();
	*this = rhs;
}// END cEncryptedTorrentConnection



// Called by PeerA when creating an outgoing connection, they have the info hash at that point.
cEncryptedTorrentConnection::cEncryptedTorrentConnection(const u8* infohash, u32 sendBufferSize, u32 recvBufferSize)
: cTcpSocket(INVALID_SOCKET, sendBufferSize, recvBufferSize)
, mState(NOT_CONNECTED)
, mWasIncomingConnection(false)
, mMseHandshakeComplete(false)
, mCryptoRecvBuffer(32 * 1024, true)
, mCryptoSendBuffer(16 * 1024, false)
, vc_off(0)
, crypto_select(0)
, pad_D_len(0)
, end_of_crypto_handshake(0)
, crypto_provide(0)
, req1_off(0)
, pad_C_len(0)
, ia_len(0)
{
	Init();

	// SKEY = Stream Identifier/Shared secret used to drop connections early if we don't have a matching stream. It's additionally used to harden the protocol
	// against MITM attacks and portscanning. Protocols w/o unique stream properties may use a constant. Note: For BitTorrent, the SKEY should be the torrent info hash.
	memcpy(SKEY, infohash, sizeof(SKEY));

	GeneratePublicPrivateKey(&Xa, &Ya);
}// END cEncryptedTorrentConnection




// Called by PeerB to create a new socket for an incoming connection, they do NOT know which info hash (ie torrent) this 
// connection is for yet, we will guess it from data in the forthcoming handshake.
cEncryptedTorrentConnection::cEncryptedTorrentConnection(u32 sendBufferSize, u32 recvBufferSize)
: cTcpSocket(INVALID_SOCKET, sendBufferSize, recvBufferSize)
, mState(WAITING_FOR_YA)
, mWasIncomingConnection(true)
, mMseHandshakeComplete(false)
, mCryptoRecvBuffer(32 * 1024, true)
, mCryptoSendBuffer(16 * 1024, false)
, vc_off(0)
, crypto_select(0)
, pad_D_len(0)
, end_of_crypto_handshake(0)
, crypto_provide(0)
, req1_off(0)
, pad_C_len(0)
, ia_len(0)
{
	Init();
	memset(SKEY, 0, sizeof(SKEY));
	GeneratePublicPrivateKey(&Xb, &Yb);
}// END cEncryptedTorrentConnection




cEncryptedTorrentConnection::~cEncryptedTorrentConnection()
{
	BN_free(&S);
	
	BN_free(&Xa);
	BN_free(&Ya);

	BN_free(&Xb);
	BN_free(&Yb);
}// END ~cEncryptedTorrentConnection



const cEncryptedTorrentConnection& cEncryptedTorrentConnection::operator= (const cEncryptedTorrentConnection& rhs)
{
	cTcpSocket::operator =(rhs);

	mState = rhs.mState;
	mWasIncomingConnection = rhs.mWasIncomingConnection;
	mMseHandshakeComplete = rhs.mMseHandshakeComplete;
	mCryptoSendBuffer = rhs.mCryptoSendBuffer;
	mCryptoRecvBuffer = rhs.mCryptoRecvBuffer;


	// NB : we don't copy P & G as they are essentially const


	BN_copy(&S, &(rhs.S));
	memcpy(SKEY, rhs.SKEY, sizeof(SKEY));


	BN_copy(&Xa, &(rhs.Xa));
	BN_copy(&Ya, &(rhs.Ya));
	vc_off = rhs.vc_off;
	crypto_select = rhs.crypto_select;
	pad_D_len = rhs.pad_D_len;
	end_of_crypto_handshake = rhs.end_of_crypto_handshake;


	BN_copy(&Xb, &(rhs.Xb));
	BN_copy(&Yb, &(rhs.Yb));
	crypto_provide = rhs.crypto_provide;
	req1_off = rhs.req1_off;
	pad_C_len = rhs.pad_C_len;
	ia_len = rhs.ia_len;


	memcpy(encryptionKey, rhs.encryptionKey, sizeof(encryptionKey));
	memcpy(decryptionKey, rhs.decryptionKey, sizeof(decryptionKey));
	memcpy(&encKey, &rhs.encKey, sizeof(RC4_KEY));
	memcpy(&decKey, &rhs.decKey, sizeof(RC4_KEY));

	return *this;
}// END operator=



void cEncryptedTorrentConnection::Init()
{
	memset(encryptionKey, 0, sizeof(encryptionKey));
	memset(decryptionKey, 0, sizeof(decryptionKey));
	memset(&encKey, 0, sizeof(RC4_KEY));
	memset(&decKey, 0, sizeof(RC4_KEY));

	BN_init(&Xa);
	BN_init(&Ya);

	BN_init(&Xb);
	BN_init(&Yb);

	BN_init(&S);
}// END Init



// Pubkey of A: Ya = (G^Xa) mod P
// Pubkey of B: Yb = (G^Xb) mod P
void cEncryptedTorrentConnection::GeneratePublicPrivateKey(BIGNUM* priv, BIGNUM* pub)
{
	// Xa and Xb are a variable size random integers. You should use a length of 160bits whenever possible
	BN_rand(priv, 160, 1, 0);

	// ^ is power !
	// BN_mod_exp() computes a to the p-th power modulo m (r=a^p % m).
	BN_mod_exp(pub, G, priv, P, gBnContext);
}// END GeneratePublicPrivateKey



// "HASH('keyA', S, SKEY)" if you're A
// "HASH('keyB', S, SKEY)" if you're B
void cEncryptedTorrentConnection::CalcEncryptionKey(bool a, const BIGNUM& s, const u8* skey, u8* keyOut)
{
	u8 buf[120];
	memcpy(buf,"key",3);
	buf[3] = (u8)(a ? 'A' : 'B');
	u32 size = BN_bn2bin(&s, buf + 4);
	memcpy(buf + 100, skey, 20);
	SHA1(buf, 120, keyOut);
}// END CalcEncryptionKey



// Calculates our encryption and decryption keys.
void cEncryptedTorrentConnection::InitEncryptionKeys(bool amPeerA)
{
	// amPeerA ensures that we get the crypto keys the right way around
	CalcEncryptionKey(amPeerA, S, SKEY, encryptionKey);
	CalcEncryptionKey(!amPeerA, S, SKEY, decryptionKey);
	RC4_set_key(&encKey, 20, encryptionKey);
	RC4_set_key(&decKey, 20, decryptionKey);

	// discard first 1024 bytes of encrypted data, i.e encrypt 1k of nothing
	u8 emptyBufferA[1024];
	memset(emptyBufferA, 0, sizeof(emptyBufferA));
	RC4(&encKey, 1024, emptyBufferA, emptyBufferA);
	RC4(&decKey, 1024, emptyBufferA, emptyBufferA);
}// END InitEncryptionKeys



/*
void cEncryptedTorrentConnection::Handshake(u8* pHandshakeMessage, u32 handshakeLength)
{
	assert(0);
	mHandshakePayload.StreamBytes(pHandshakeMessage, handshakeLength);
	mState = CONNECTING;
}// END Handshake
*/


bool cEncryptedTorrentConnection::OpenAndConnect(const cSockAddr& addr, bool bBlocking)
{
	// First PeerA state
	mState = CONNECTING;
	return cTcpSocket::OpenAndConnect(addr, bBlocking);
}// END OpenAndConnect



bool cEncryptedTorrentConnection::Close()
{
	mState = NOT_CONNECTED;
	mCryptoSendBuffer.Clear(true);
	mCryptoRecvBuffer.Clear(true);
	mMseHandshakeComplete = false;
	return cTcpSocket::Close();
}// END Close



void cEncryptedTorrentConnection::Process()
{
#if 0
	if(mCryptoRecvBuffer.Capacity() >= 1024*1024*2)
	{
		Printf("MSE: *WARNING* Huge mCryptoRecvBuffer %.2fMb\n", float(mCryptoRecvBuffer.Capacity() / (1024.0f * 1024.0f)));
		//assert(0);
	}
#endif


	// No processing to do once we are connected.
	if(mState == CONNECTED_PEERA || mState == CONNECTED_PEERB || !IsOpen())
	{
		return;
	}


	switch(mState)
	{
	case NOT_CONNECTED:
		break;

	//////////////////////////////////////////////////////////////////////////
	// PeerA States
	case CONNECTING:
		if(IsOpen() && cTcpSocket::ConnectionEstablished())
		{
			SendPublicKeyPlusPadding(Ya);
			mState = SENT_YA;
		}
		break;

	case SENT_YA:
		HandleYb();
		break;

	case GOT_YB:
		FindVC();
		break;

	case FOUND_VC:
		HandleCryptoSelect();
		break;

	case WAIT_FOR_PAD_D:
		HandlePadD();
		break;


	//////////////////////////////////////////////////////////////////////////
	// PeerB States
	case WAITING_FOR_YA:
		HandleYa();
		break;

	case WAITING_FOR_REQ1:
		FindReq1();
		break;

	case FOUND_REQ1:
		CalculateSKey();
		break;

	case FOUND_INFO_HASH:
		ProcessVC();
		break;

	case WAIT_FOR_PAD_C:
		HandlePadC();
		break;

	case WAIT_FOR_IA:
		HandleIA();
		break;



	default:
		assert(0);
	}
}// END Process



s32 cEncryptedTorrentConnection::Recv(void* pBuffer, u32 bufSize, bool peak) const
{
	if(bufSize == 0 || pBuffer == NULL)
	{
		assert(0);
		return 0;
	}

	// not finished handshake yet, pass all data straight through
	if(mMseHandshakeComplete == false)
	{
		return cTcpSocket::Recv(pBuffer, bufSize, peak);
	}

	if(crypto_select == CRYPTO_MODE_RC4)
	{
		u32 previousSize = mCryptoRecvBuffer.Size();

		
		u32 bytesPending = BytesPendingOnInputBuffer();
		if(mCryptoRecvBuffer.FreeSpace() < bytesPending)
		{
			mCryptoRecvBuffer.DoubleSize();
		}
		
		// we always read and decrypt any pending data, it is then held in mCryptoRecvBuffer (decrypted) and fed back to from there
		s32 bytesRcvd = cTcpSocket::Recv(mCryptoRecvBuffer.Data() + mCryptoRecvBuffer.Size(), mCryptoRecvBuffer.FreeSpace(), false);		
		
		// We can only return the error code after we have drained all the buffered data we have.
		// This is working under the assumption that the error code will continue to return the same value.
		if(bytesRcvd < 0 && mCryptoRecvBuffer.Size() == 0)
		{
			return bytesRcvd;
		}

		// keep the size up to date
		if(bytesRcvd > 0)
		{
			mCryptoRecvBuffer.BytesStreamedManually(bytesRcvd);
		}

		if(mCryptoRecvBuffer.Size() == 0)
		{
			return 0;
		}

		// decrypt everything pending on the input buffer
		RC4(&decKey, bytesRcvd, mCryptoRecvBuffer.Data() + previousSize, mCryptoRecvBuffer.Data() + previousSize);
	
		u32 numBytes = min(mCryptoRecvBuffer.Size(), bufSize);
		memcpy(pBuffer, mCryptoRecvBuffer.Data(), numBytes);

		if(peak==false)
		{
			mCryptoRecvBuffer.RemoveBytes(0, numBytes);
		}
		return numBytes;	
	}
	else
	{
		assert(crypto_select == CRYPTO_MODE_PLAIN_TEXT);
		assert(mCryptoRecvBuffer.Size() == 0 || mCryptoRecvBuffer.Size() == HANDSHAKE_LENGTH);
		if(mCryptoRecvBuffer.Size() > 0)
		{
			if(bufSize < HANDSHAKE_LENGTH)
			{
				assert(0);
				return 0;
			}
			u32 numBytes = min(mCryptoRecvBuffer.Size(), bufSize);
			memcpy(pBuffer, mCryptoRecvBuffer.Data(), numBytes);
			if(peak==false)
			{
				mCryptoRecvBuffer.RemoveBytes(0, numBytes);
			}
			return numBytes;	
		}
		return cTcpSocket::Recv(pBuffer, bufSize, peak);
	}
}// END Recv



s32 cEncryptedTorrentConnection::Send(const void* pPacket, u32 packetSize)
{
	// not finished handshake yet, pass all data straight through
	if(mMseHandshakeComplete == false)
	{
		return cTcpSocket::Send(pPacket, packetSize);
	}

	// If we need to, decrypt the data ...
	if(crypto_select == CRYPTO_MODE_RC4)
	{
		mCryptoSendBuffer.Clear(false);
		mCryptoSendBuffer.StreamBytes(reinterpret_cast<const u8*> (pPacket), packetSize);
		RC4(&encKey, packetSize, mCryptoSendBuffer.Data(), mCryptoSendBuffer.Data());
		return cTcpSocket::Send(mCryptoSendBuffer.Data(), packetSize);
	}
	else
	{
		assert(crypto_select == CRYPTO_MODE_PLAIN_TEXT);
		return cTcpSocket::Send(pPacket, packetSize);
	}	
}// END Send



bool cEncryptedTorrentConnection::ConnectionEstablished() const
{
	bool tcpEst = cTcpSocket::ConnectionEstablished();
	return (tcpEst && mMseHandshakeComplete == true);
}// END ConnectionEstablished



u32 cEncryptedTorrentConnection::BytesPendingOnInputBuffer() const
{
	return cTcpSocket::BytesPendingOnInputBuffer() + mCryptoRecvBuffer.Size();
}// END BytesPendingOnInputBuffer



void cEncryptedTorrentConnection::SendPublicKeyPlusPadding(const BIGNUM& publicKey)
{
	u8 tmp[608];
	u32 length = BN_bn2bin(&publicKey, tmp);
	s32 bytesSent = Send(tmp, 96 + rand() % 512);
}// END SendPublicKeyPlusPadding




//////////////////////////////////////////////////////////////////////////
// PeerA States


void cEncryptedTorrentConnection::HandleYb()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	if(bytesRecv >=  96)
	{
		//Printf("MSE: Got Yb\n");

		// 2 B->A: Diffie Hellman Yb, PadB	
		BN_bin2bn(mCryptoRecvBuffer.Data(), 96, &Yb);

		// DH secret: S = (Ya^Xb) mod P = (Yb^Xa) mod P
		BN_mod_exp(&S, &Yb, &Xa, P, gBnContext);


		mState = GOT_YB;


		// now we must send line 3 
		// 3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)

		u32 length;
		u8 tmp_buf[120]; // temporary buffer
		u8 h1[20], h2[20]; // temporary hash

		// generate and send the first hash
		memcpy(tmp_buf,"req1",4);
		length = BN_bn2bin(&S, tmp_buf + 4);
		SHA1(tmp_buf, 100, h1);
		Send(h1, 20);

		// generate second and third hash and xor them
		memcpy(tmp_buf,"req2",4);
		memcpy(tmp_buf+4, SKEY, 20);
		SHA1(tmp_buf, 24, h1);


		memcpy(tmp_buf,"req3",4);
		length = BN_bn2bin(&S, tmp_buf + 4);
		SHA1(tmp_buf, 100, h2);

		// xor them
		u8 xorHash[20];
		for(u32 i=0; i < 20; i++)
		{
			xorHash[i] = (h1[i] ^ h2[i]);
		}
		Send(xorHash, 20);

		
		// now we can calculate our encryption and decryption keys
		InitEncryptionKeys(true);


		// Now send ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA))
		memset(tmp_buf,0,16); // VC are 8 0x00's
		
		
		if(BitTorrentManager().TorrentConnectionsMustBeEncrypted(SKEY))
		{
			tmp_buf[11] = CRYPTO_MODE_RC4;			
		}
		else
		{
			tmp_buf[11] = CRYPTO_MODE_PLAIN_TEXT | CRYPTO_MODE_RC4; // we support both plain text and rc4
		}
		
		// no padC
		u16 lenPadC = 0x0000;
		endian_swap(lenPadC);
		memcpy(tmp_buf + 12, &lenPadC, 2);

		// length of IA, which will be the bittorrent handshake
		u16 lenIA = (u16) 0; //mHandshakePayload.Size();
		// handshake or nothing !
		assert(lenIA == HANDSHAKE_LENGTH || lenIA == 0);
		endian_swap(lenIA);
		memcpy(tmp_buf + 14, &lenIA, 2);

#if 0
		if(mHandshakePayload.Size() > 0)
		{
			// I've decided against piggybacking the handshake so i can make the encryption layer transparent
			assert(0);

			// send IA which is the handshake		
			memcpy(tmp_buf+16, mHandshakePayload.Data(), mHandshakePayload.Size());
			assert(mHandshakePayload.Size() + 16 == 84);

			// encrypt & send
			u8 encData[84];
			RC4(&encKey, 84, tmp_buf, encData);
			Send(encData,84); 
		}
		else
#endif
		{
			// encrypt & send
			u8 encData[16];
			RC4(&encKey, 16, tmp_buf, encData);
			Send(encData, 16); 
		}
	}
}// END HandleYb



void cEncryptedTorrentConnection::FindVC()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	if(bytesRecv > 0)
	{
		u8 vc[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

		// create a copy of the othersides encryption key (our decryption key) so we don't fuck over our real key
		RC4_KEY decKeyCopy;
		memcpy(&decKeyCopy, &decKey, sizeof(RC4_KEY));
		RC4(&decKeyCopy, 8, vc, vc);

		u8* buf = mCryptoRecvBuffer.Data();
		bool foundVc=false;
		u32 max_i = mCryptoRecvBuffer.Size() - 8;	
		for (u32 i = 96; i < max_i; i++)
		{	
			if (vc[0] == buf[i] && memcmp(buf+i,vc,8) == 0)
			{
				//state = FOUND_VC;
				vc_off = i;
				foundVc = true;
				mState = FOUND_VC;
				return;
			}
		}

		// we haven't found it in the first 616 bytes (96 + max 512 padding + 8 bytes VC)
		if (!foundVc && mCryptoRecvBuffer.Size() >= 616)
		{
			Printf("MSE: No VC, invalid connection\n");
			Close();
			return;
		}
	}
}// END FindVC



void cEncryptedTorrentConnection::HandleCryptoSelect()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	// Not enough data available yet
	if (vc_off + 14 >= mCryptoRecvBuffer.Size())
	{
		return;
	}

	u8* buf = mCryptoRecvBuffer.Data();

	// now decrypt the first 14 bytes
	RC4(&decKey, 14, mCryptoRecvBuffer.Data() + vc_off, mCryptoRecvBuffer.Data() + vc_off);

	// check the VC, should be all zero
	for (u32 i = vc_off;i < vc_off + 8;i++)
	{
		if (buf[i] != 0)
		{
			// invalid vc
			Close();
			assert(0);
			return;
		}
	}

	crypto_select = mCryptoRecvBuffer.ReadU32(vc_off + 8);
	endian_swap(crypto_select);
	if(crypto_select != CRYPTO_MODE_PLAIN_TEXT && crypto_select != CRYPTO_MODE_RC4)
	{
		Printf("MSE: *WARNING* Invalid crypto select: %u\n", crypto_select);
		//assert(0);
		Close();
		return;
	}
	pad_D_len = mCryptoRecvBuffer.ReadU16(vc_off + 12);
	endian_swap(pad_D_len);
	if (pad_D_len > 512)
	{
		// invalid
		Close();
		assert(0);
		return;
	}

	end_of_crypto_handshake = vc_off + 14 + pad_D_len;

	mState = WAIT_FOR_PAD_D;
}// END HandleCryptoSelect



void cEncryptedTorrentConnection::HandlePadD()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	if(mCryptoRecvBuffer.Size() < end_of_crypto_handshake)
	{
		// TODO : timeout
		return;
	}

	if(pad_D_len > 0)
	{
		// decrypt the padding
		RC4(&decKey, pad_D_len, mCryptoRecvBuffer.Data() + vc_off + 14, mCryptoRecvBuffer.Data() + vc_off + 14);
	}

	// This should now point us at the standard bittorrent stream
	mCryptoRecvBuffer.RemoveBytes(0, end_of_crypto_handshake);
	mMseHandshakeComplete = true;

	if (crypto_select & CRYPTO_MODE_PLAIN_TEXT)
	{
		//Printf("MSE: *WARNING* Plain Text selected.\n");
	}
	// now it must be rc4 if not exit
	else if (crypto_select & CRYPTO_MODE_RC4) 
	{
		//Printf("MSE: RC4 Encryption selected.\n");
		
		// decrypt everything pending on the input buffer
		RC4(&decKey, mCryptoRecvBuffer.Size(), mCryptoRecvBuffer.Data(), mCryptoRecvBuffer.Data());
	}
	else 
	{
		Printf("MSE: Unsupported crypto method, aborting connection.\n");
		Close();
		assert(0);
		return;
	}

	mState = CONNECTED_PEERA;
}// END HandlePadD

// END PeerA States
//////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////
// PeerB States


void cEncryptedTorrentConnection::HandleYa()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	if(bytesRecv >=  96)
	{
		//Printf("MSE: Got Ya\n");

		SendPublicKeyPlusPadding(Yb);

		// 1 A->B: Diffie Hellman Ya, PadA
		BN_bin2bn(mCryptoRecvBuffer.Data(), 96, &Ya);

		// DH secret: S = (Ya^Xb) mod P = (Yb^Xa) mod P
		BN_mod_exp(&S, &Ya, &Xb, P, gBnContext);
		

		mState = WAITING_FOR_REQ1;
	}

	// TODO : timeout

}// END HandleYa



void cEncryptedTorrentConnection::FindReq1()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	if(mCryptoRecvBuffer.Size() >=  116)
	{
		u8 tmp[100];
		u8 req1Hash[20];
		u32 length;

		memcpy(tmp, "req1", 4);
		length = BN_bn2bin(&S, tmp + 4);
		SHA1(tmp, 100, req1Hash);
	
		u8* buf = mCryptoRecvBuffer.Data();
		for (u32 i = 96;i < mCryptoRecvBuffer.Size() - 20; i++)
		{
			if (buf[i] == req1Hash[0] && memcmp(buf+i, req1Hash, 20) == 0)
			{
				mState = FOUND_REQ1;
				req1_off = i;
				return;
			}
		}
	}
	
	if (mCryptoRecvBuffer.Size() > 608)
	{
		// connection is not valid, probably a connection from a previous run with different keys.
		Printf("MSE: Received invalid encrypted connection\n");
		Close();
	}
}// END FindReq1



void cEncryptedTorrentConnection::CalculateSKey()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	// We need to have the two 20 byte hashes to proceed
	if(mCryptoRecvBuffer.Size() >= (req1_off + 40))
	{
		u8 tmp[100];
		u8 xordHash[20];
		u8 req2Hash[20];
		u8 req3Hash[20];
		u32 length;

		// calculate the 'req3' hash
		memcpy(tmp,"req3",4);
		length = BN_bn2bin(&S, tmp + 4);
		SHA1(tmp, 100, req3Hash);
		
		// copy in the xor'd hash the otherside has sent us
		memcpy(xordHash, mCryptoRecvBuffer.Data() + req1_off + 20, 20);

		// xordHash = HASH('req2', SKEY) xor HASH('req3', S)
		// now calculate HASH('req2', SKEY)
		for(u32 i=0; i < 20; i++)
		{
			req2Hash[i] = (xordHash[i] ^ req3Hash[i]);
		}

		const u8* infoHash = BitTorrentManager().GetTorrentInfoHashFromReq2Hash(req2Hash);
		if(infoHash == NULL)
		{
			// info hash not found, we are not participating in this torrent
			Printf("MSE: Incoming connection for unknown info hash, connection refused.\n");			
			Close();
			return;
		}

		memcpy(SKEY, infoHash, INFO_HASH_LENGTH);
		mState = FOUND_INFO_HASH;
	}
}// END CalculateSKey



void cEncryptedTorrentConnection::ProcessVC()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	// We need to have the two 20 byte hashes and the 14 bytes ENCRYPT(VC, crypto_provide, len(PadC)) to proceed
	if(mCryptoRecvBuffer.Size() >= (req1_off + 40 + 14))
	{
		// now we can calculate our encryption and decryption keys
		InitEncryptionKeys(false);

		u8* buf = mCryptoRecvBuffer.Data();

		u32 vc_off = req1_off + 40;
		// now decrypt the vc and crypto_provide and the length of pad_C
		RC4(&decKey, 14, mCryptoRecvBuffer.Data() + vc_off, mCryptoRecvBuffer.Data() + vc_off);

		// check the VC
		for (u32 i = 0; i < 8; i++)
		{
			if (buf[vc_off + i] != 0)
			{
				Printf("MSE: VC should be all zero, abort connection\n");
				//assert(0);				
				Close();
				return;
			}
		}
		// get crypto_provide and the length of pad_C
		crypto_provide = mCryptoRecvBuffer.ReadU32(vc_off + 8);
		endian_swap(crypto_provide);

		pad_C_len = mCryptoRecvBuffer.ReadU16(vc_off + 12);
		endian_swap(pad_C_len);
		if (pad_C_len > 512)
		{
			Printf("MSE: Illegal padC length, abort connection\n");
			//assert(0);				
			Close();
			return;
		}

		// Send - ENCRYPT(VC, crypto_select, len(padD), padD)
		u8 tmp[14];
		memset(tmp,0,14); // VC
		if (crypto_provide & CRYPTO_MODE_RC4) // RC4 
		{
			//Printf("MSE: RC4 Encryption selected (by us).\n");
			crypto_select = CRYPTO_MODE_RC4;
		}
		else
		{
			//Printf("MSE: *WARNING* Client does not support encrypted transport.\n");
			
			if(BitTorrentManager().TorrentConnectionsMustBeEncrypted(SKEY))
			{
				// Should never get here, invalid select
				assert(0);			
				Close();
				return;
			}
			
			// if we want to allow ...
			crypto_select = CRYPTO_MODE_PLAIN_TEXT;
		}
		u32 bigEndianCryptoSelect = crypto_select;
		endian_swap(bigEndianCryptoSelect);
		memcpy(tmp + 8, &bigEndianCryptoSelect, 4);	

		// no pad D
		u16 lenPadD = 0;
		memcpy(tmp + 12, &lenPadD, 2);

		// encrypt and send
		RC4(&encKey, 14, tmp, tmp);
		Send(tmp, 14);

		mState = WAIT_FOR_PAD_C;
	}
}// END ProcessVC



void cEncryptedTorrentConnection::HandlePadC()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	// We need to have the two 20 byte hashes and the 14 bytes ENCRYPT(VC, crypto_provide, len(PadC)) and padC and len(IA) to proceed
	if(mCryptoRecvBuffer.Size() >= (req1_off + 54 + pad_C_len + 2))
	{
		// We have already decrypted everything up to and including len(PadC)
		u32 padcOff = req1_off + 54;
		
		// decrypt PadC and len(IA)
		RC4(&decKey, pad_C_len + 2, mCryptoRecvBuffer.Data() + padcOff, mCryptoRecvBuffer.Data() + padcOff);

		memcpy(&ia_len, mCryptoRecvBuffer.Data() + padcOff + pad_C_len, 2);
		endian_swap(ia_len);
		if(ia_len != 0 && ia_len != HANDSHAKE_LENGTH)
		{			
			Printf("MSE: Incoming crypto connection has a payload that is not the handshake, aborting connection\n");
			//assert(0);
			Close();
			return;
		}

		mState = WAIT_FOR_IA;
	}
}// END HandlePadC



void cEncryptedTorrentConnection::HandleIA()
{
	s32 bytesRecv = Recv(&mCryptoRecvBuffer, mCryptoRecvBuffer.Capacity());
	if(bytesRecv < 0)
	{
		Close();
		return;
	}

	// We need to have received everything from step 3 of the handshake to proceed:
	// 3 A->B: HASH('req1', S), HASH('req2', SKEY) xor HASH('req3', S), ENCRYPT(VC, crypto_provide, len(PadC), PadC, len(IA)), ENCRYPT(IA)
	if(mCryptoRecvBuffer.Size() >= (req1_off + 54 + pad_C_len + 2 + ia_len))
	{
		// Handshake complete, throw away the handshake data, leaving only the payload (if any)
		mCryptoRecvBuffer.RemoveBytes(0, req1_off + 54 + pad_C_len + 2);
		mMseHandshakeComplete = true;

		// handshake or nothing (can other messages be queued up now??)
		if(mCryptoRecvBuffer.Size() != 0 && mCryptoRecvBuffer.Size() != HANDSHAKE_LENGTH)
		{
			Printf("MSE: Payload is not zero or handshake sized, abort connection.\n");
			//assert(0);
			Close();
			return;
		}
		

		// Decrypt whatever bytes we have (payload(IA) and any other messages that have started to arrive)
		if (mCryptoRecvBuffer.Size() > 0)
		{
			RC4(&decKey, mCryptoRecvBuffer.Size(), mCryptoRecvBuffer.Data(), mCryptoRecvBuffer.Data());
		}


		mState = CONNECTED_PEERB;
	}
}// END HandleIA





// END PeerB States
//////////////////////////////////////////////////////////////////////////




