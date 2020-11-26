// Jon Bellamy 21/02/2009


#ifndef BITTORRENT_MESSAGES_H
#define BITTORRENT_MESSAGES_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <assert.h>

#include "General/Endianness.h"
#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/BitTorrent/BitTorrentPeer.h"


/////////////////////////////////////////////////////////////
// Message Def's


// byte-align structures
#ifdef _MSC_VER
#	pragma pack( push, packing )
#	pragma pack( 1 )
#	define PACK_STRUCT
#elif defined( __GNUC__ )
#	define PACK_STRUCT	__attribute__((packed))
#else
#	error you must byte-align these structures with the appropriate compiler directives
#endif




// special case message, does not have the length and message id prefix
class cTorrentHandshakeMessage
{
public:
	cTorrentHandshakeMessage() {}
	cTorrentHandshakeMessage(const u8* infoHash, const u8* peerId);

	bool IsValid() const;

	enum
	{
		PROTO_STR_LENGTH = 19
	};

	u8 mProtocolStrLen;		// In version 1.0 of the BitTorrent protocol, pstrlen = 19, and pstr = "BitTorrent protocol". 
	u8 mProtocolId[PROTO_STR_LENGTH];
	u8 mReserved[8];
	u8 mInfoHash[INFO_HASH_LENGTH];
	u8 mPeerId[PEER_ID_LENGTH];
};


// another (kind of) special case, the keep-alive message is a message with zero bytes, specified 
// with the length prefix set to zero. There is no message ID and no payload.
class cTorrentKeepAliveMessage
{
public:
	cTorrentKeepAliveMessage()
	{
		mSize=0;
	}

	u32 mSize;
};



// All of the remaining messages in the protocol take the form of <length prefix><message ID><payload>. 
// The length prefix is a four byte big-endian value. The message ID is a single decimal byte.


typedef enum
{
	TM_CHOKE = 0,
	TM_UNCHOKE,
	TM_INTERESTED,
	TM_NOT_INTERESTED,
	TM_HAVE,
	TM_BITFIELD,
	TM_REQUEST,
	TM_PIECE,
	TM_CANCEL,
	TM_PORT,
	NUM_TORRENT_MESSAGE_TYPES		// this does not include keep alive or handshake
}TorrentMessageId;



class cTorrentMessage
{
public:
	cTorrentMessage(u32 size, TorrentMessageId id) : mSize(size), mMessageId(id) {}

	TorrentMessageId Type () const { return static_cast<TorrentMessageId> (mMessageId); }

	static u32 HeadSize() { return sizeof(u32); }

	void OnReceive() { endian_swap(mSize); }

	u32 mSize;

	// size counts from here ...
	u8 mMessageId;
};



// The choke message is fixed-length and has no payload
class cTorrentMessageChoke : public cTorrentMessage
{
public:
	cTorrentMessageChoke() : cTorrentMessage(1, TM_CHOKE) {}
};



// The unchoke message is fixed-length and has no payload
class cTorrentMessageUnchoke : public cTorrentMessage
{
public:
	cTorrentMessageUnchoke () : cTorrentMessage(1, TM_UNCHOKE) {}
};



// The interested message is fixed-length and has no payload
class cTorrentMessageInterested : public cTorrentMessage
{
public:
	cTorrentMessageInterested () : cTorrentMessage(1, TM_INTERESTED) {}
};



// The not interested message is fixed-length and has no payload
class cTorrentMessageNotInterested : public cTorrentMessage
{
public:
	cTorrentMessageNotInterested () : cTorrentMessage(1, TM_NOT_INTERESTED) {}
};



// The have message is fixed length. The payload is the zero-based index of a piece that has just been 
// successfully downloaded and verified via the hash
class cTorrentMessageHave : public cTorrentMessage
{
public:
	cTorrentMessageHave () : cTorrentMessage(5, TM_HAVE) {}

	u32 mPieceIndex;
};



// The bitfield message may only be sent immediately after the handshaking sequence is completed, and before 
// any other messages are sent. It is optional, and need not be sent if a client has no pieces. It is 
// variable length
class cTorrentMessageBitfield : public cTorrentMessage
{
public:
	cTorrentMessageBitfield (u32 bitfieldSize) : cTorrentMessage(1 + bitfieldSize, TM_BITFIELD) {}

	//u8 bits[1];
};



// The request message is fixed length, and is used to request a block. The payload contains the following information:
//  * index: integer specifying the zero-based piece index
//  * begin: integer specifying the zero-based byte offset within the piece
//  * length: integer specifying the requested length. 
class cTorrentMessageRequest : public cTorrentMessage
{
public:
	cTorrentMessageRequest () : cTorrentMessage(13, TM_REQUEST) {}

	u32 mPieceIndex;
	u32 mBegin;
	u32 mLength;
};




//The piece message is variable length, where X is the length of the block. The payload contains the following information:
//  * index: integer specifying the zero-based piece index
//  * begin: integer specifying the zero-based byte offset within the piece
//  * block: block of data, which is a subset of the piece specified by index. 
class cTorrentMessagePiece : public cTorrentMessage
{
public:
	cTorrentMessagePiece (u32 dataSize) : cTorrentMessage(9 + dataSize, TM_PIECE) {}

	u32 mPieceIndex;
	u32 mBegin;

	// there then follows the payload bytes
	//u8 data[1];
};



// The cancel message is fixed length, and is used to cancel block requests. The payload is identical to that 
// of the "request" message. It is typically used during "End Game" (see the Algorithms section below). 
class cTorrentMessageCancel : public cTorrentMessage
{
public:
	cTorrentMessageCancel () : cTorrentMessage(13, TM_CANCEL) {}

	u32 mPieceIndex;
	u32 mBegin;
	u32 mLength;
};



// The port message is sent by newer versions of the Mainline that implements a DHT tracker. The listen port is 
// the port this peer's DHT node is listening on. This peer should be inserted in the local routing 
// table (if DHT tracker is supported). 
class cTorrentMessagePort : public cTorrentMessage
{
public:
	cTorrentMessagePort () : cTorrentMessage(3, TM_PORT) {}

	u16 mPort;
};



// Default alignment
#ifdef _MSC_VER
#	pragma pack( pop, packing )
#endif

#undef PACK_STRUCT





/////////////////////////////////////////////////////////////
// Message Sending & Message Handlers


class cBitTorrent;


extern bool SendHandshakeMessage(cTorrentPeer& peer, const u8* infoHash, const u8* peerId);

extern bool SendKeepAlive(cTorrentPeer& peer);
extern bool SendChokeMessage(cTorrentPeer& peer);
extern bool SendUnChokeMessage(cTorrentPeer& peer);
extern bool SendInterestedMessage(cTorrentPeer& peer);
extern bool SendNotInterestedMessage(cTorrentPeer& peer);
extern bool SendHaveMessage(cTorrentPeer& peer, u32 pieceIndex);
extern bool SendBitfieldMessage(cTorrentPeer& peer, const cBitField& bitfield);
extern bool SendRequestMessage(cTorrentPeer& peer, u32 pieceIndex, u32 begin, u32 length);
extern bool SendPieceMessage(cBitTorrent* pTorrent, cTorrentPeer& peer, u32 pieceIndex, u32 begin, u32 length, const u8* bytes);
extern bool SendCancelMessage(cTorrentPeer& peer, u32 pieceIndex, u32 begin, u32 length);
extern bool SendPortMessage(cTorrentPeer& peer);

extern void MessageHandler_KeepAlive(cTorrentPeer& peer, cBitTorrent* pTorrent);
extern void MessageHandler_Choke(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Unchoke(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Interested(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_NotInterested(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Have(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Bitfield(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Request(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Piece(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Cancel(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);
extern void MessageHandler_Port(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);





#endif // BITTORRENT_MESSAGES_H
