// Jon Bellamy 21/02/2009


#include "BitTorrentMessages.h"

#include <memory.h>

#include "Network/BitTorrent/BitTorrent.h"


const char gProtocolName[] = "BitTorrent protocol";


/////////////////////////////////////////////////////////////
// Messages

cTorrentHandshakeMessage::cTorrentHandshakeMessage(const u8* infoHash, const u8* peerId)
{
	mProtocolStrLen = PROTO_STR_LENGTH;
	memcpy(mProtocolId, gProtocolName, PROTO_STR_LENGTH);

	memset(mReserved, 0, sizeof(mReserved));

	memcpy(mInfoHash, infoHash, INFO_HASH_LENGTH);
	memcpy(mPeerId, peerId, PEER_ID_LENGTH);
}// END cTorrentHandshakeMessage



bool cTorrentHandshakeMessage::IsValid() const
{
	return memcmp(mProtocolId, gProtocolName, PROTO_STR_LENGTH)==0;
}// END IsValid



bool SendHandshakeMessage(cTorrentPeer& peer, const u8* infoHash, const u8* peerId)
{
	//Printf("SendHandshakeMessage\n");

	peer.OnMessageSent();

	cTorrentHandshakeMessage m(infoHash, peerId);
	u32 size = static_cast<u32> (sizeof(cTorrentHandshakeMessage));
	
	
#if 0
	//cMse::ClientConnection(peer.Torrent()->InfoHash(), peer.Socket(), reinterpret_cast<u8*>(&m), size);
	// This kicks off the MSE handshake, which will in turn send the bittorrent handshake (when its ready)
	peer.Socket().Handshake(reinterpret_cast<u8*>(&m), size);
#else	
	peer.Socket().Send(&m, size);
#endif

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::HANDSHAKE, PTYPE_SEND);

	peer.HandshakeSent(true);

	return true;
}// END SendHandshakeMessage



bool SendKeepAlive(cTorrentPeer& peer)
{
	//Printf("SendKeepAlive to %s\n", peer.Address().Ip().AsString());

	peer.OnMessageSent();

	cTorrentKeepAliveMessage m;
	// size is zero so no need for endianswap

	peer.LogNote(0, "KEEP ALIVE SENT");
	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentKeepAliveMessage));

	return bytesSent == sizeof(cTorrentKeepAliveMessage);
}// END SendKeepAlive



bool SendChokeMessage(cTorrentPeer& peer)
{
	//Printf("SendChokeMessage %s\n", peer.Address().Ip().AsString());

	peer.OnMessageSent();

	cTorrentMessageChoke m;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageChoke));
	peer.AmChoking(true);

	return bytesSent == sizeof(cTorrentMessageChoke);
}// END SendChokeMessage



bool SendUnChokeMessage(cTorrentPeer& peer)
{
	//Printf("SendUnChokeMessage %s\n", peer.Address().Ip().AsString());

	peer.OnMessageSent();

	cTorrentMessageUnchoke m;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageUnchoke));
	peer.AmChoking(false);

	return bytesSent == sizeof(cTorrentMessageUnchoke);
}// END SendUnChokeMessage



bool SendInterestedMessage(cTorrentPeer& peer)
{
	//Printf("SendInterestedMessage\n");

	peer.OnMessageSent();

	cTorrentMessageInterested m;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageInterested));
	peer.AmInterested(true);

	return bytesSent == sizeof(cTorrentMessageInterested);
}// END SendInterestedMessage



bool SendNotInterestedMessage(cTorrentPeer& peer)
{
	//Printf("SendNotInterestedMessage\n");

	peer.OnMessageSent();

	cTorrentMessageNotInterested m;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageNotInterested));
	peer.AmInterested(false);

	return bytesSent == sizeof(cTorrentMessageNotInterested);
}// END SendNotInterestedMessage



bool SendHaveMessage(cTorrentPeer& peer, u32 pieceIndex)
{
	//Printf("SendHaveMessage\n");

	peer.OnMessageSent();

	cTorrentMessageHave m;
	m.mPieceIndex = pieceIndex;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);
	endian_swap(m.mPieceIndex);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageHave));

	return bytesSent == sizeof(cTorrentMessageHave);
}// END SendHaveMessage



bool SendBitfieldMessage(cTorrentPeer& peer, const cBitField& bitfield)
{
	//Printf("SendBitfieldMessage\n");

	peer.OnMessageSent();

	cTorrentMessageBitfield m(bitfield.BytesRequired());

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);

	if(peer.Socket().Send(&m, sizeof(cTorrentMessageBitfield)) != sizeof(cTorrentMessageBitfield))
	{
		return false;
	}

	if(peer.Socket().Send(bitfield.Storage(), bitfield.BytesRequired()) != bitfield.BytesRequired())
	{
		return false;
	}

	return true;
}// END SendBitfieldMessage



bool SendRequestMessage(cTorrentPeer& peer, u32 pieceIndex, u32 begin, u32 length)
{
	//Printf("SendRequestMessage\n");

	peer.OnMessageSent();

	cTorrentMessageRequest m;
	m.mPieceIndex = pieceIndex;
	m.mBegin = begin;
	m.mLength = length;

	assert(m.mLength <= BLOCK_LENGTH);

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);
	endian_swap(m.mPieceIndex);
	endian_swap(m.mBegin );
	endian_swap(m.mLength);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageRequest));

	return bytesSent == sizeof(cTorrentMessageRequest);
}// END SendRequestMessage



bool SendPieceMessage(cBitTorrent* pTorrent, cTorrentPeer& peer, u32 pieceIndex, u32 begin, u32 length, const u8* bytes)
{
	//Printf("Send Piece to %s\n", peer.Address().Ip().AsString());

	peer.OnMessageSent();

	cTorrentMessagePiece m(length);
	m.mPieceIndex = pieceIndex;
	m.mBegin = begin;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);
	endian_swap(m.mPieceIndex);
	endian_swap(m.mBegin );

	s32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessagePiece));
	if(bytesSent != sizeof(cTorrentMessagePiece))
	{
		Printf("send piece failed %d!!!\n", bytesSent);
		return false;
	}

	bytesSent = peer.Socket().Send(bytes, length);
	if(bytesSent != length)
	{
		Printf("send piece failed %d!!!\n", bytesSent);
		return false;
	}

	return true;
}// END SendPieceMessage



bool SendCancelMessage(cTorrentPeer& peer, u32 pieceIndex, u32 begin, u32 length)
{
	//Printf("SendCancelMessage\n");

	peer.OnMessageSent();

	cTorrentMessageCancel m;
	m.mPieceIndex = pieceIndex;
	m.mBegin = begin;
	m.mLength = length;

	peer.LogStandardMessage(0, &m, cTorrentPacketLog::STANDARD_MESSAGE, PTYPE_SEND);

	endian_swap(m.mSize);
	endian_swap(pieceIndex);
	endian_swap(begin);
	endian_swap(length);

	u32 bytesSent = peer.Socket().Send(&m, sizeof(cTorrentMessageCancel));

	return bytesSent == sizeof(cTorrentMessageCancel);
}// END SendCancelMessage



bool SendPortMessage(cTorrentPeer& peer)
{
	Printf("SendPortMessage\n");

	peer.OnMessageSent();

	assert(0);
	return false;
}// END SendCancelMessage




/////////////////////////////////////////////////////////////
// Message Handlers


void MessageHandler_KeepAlive(cTorrentPeer& peer, cBitTorrent* pTorrent)
{
	//Printf("MessageHandler_KeepAlive\n");
	peer.LogNote(0, "KEEP ALIVE RECEIVED");
}// END MessageHandler_KeepAlive



void MessageHandler_Choke(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Choke\n");
	peer.IsChokingMe(true);
}// END MessageHandler_Choke



void MessageHandler_Unchoke(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Unchoke\n");
	peer.IsChokingMe(false);
}// END MessageHandler_Unchoke



void MessageHandler_Interested(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Interested\n");
	peer.IsInterestedInMe(true);
}// END MessageHandler_Interested



void MessageHandler_NotInterested(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_NotInterested\n");
	peer.IsInterestedInMe(false);
}// END MessageHandler_NotInterested



void MessageHandler_Have(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Have\n");

	cTorrentMessageHave& message = static_cast<cTorrentMessageHave&> (msg);

	endian_swap(message.mPieceIndex);

	peer.Bitfield().Set(message.mPieceIndex);
}// END MessageHandler_Have



void MessageHandler_Bitfield(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Bitfield\n");

	const cTorrentMessageBitfield& message = static_cast<const cTorrentMessageBitfield&> (msg);

	const u8* pBits = reinterpret_cast<const u8*> ((const u8*)(&message) + sizeof(cTorrentMessageBitfield));
	cBitField bitfield;
	bitfield.SetFromData(pTorrent->FileSet().NumberOfPieces(), 1, pBits);
	peer.OnBitfieldReceived(bitfield);
}// END MessageHandler_Bitfield



void MessageHandler_Request(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Request\n");

	cTorrentMessageRequest& message = static_cast<cTorrentMessageRequest&> (msg);

	endian_swap(message.mPieceIndex);
	endian_swap(message.mBegin);
	endian_swap(message.mLength);

	peer.OnBlockRequested(pTorrent, message.mPieceIndex, message.mBegin, message.mLength);
}// END MessageHandler_Request



void MessageHandler_Piece(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	//Printf("MessageHandler_Piece\n");

	cTorrentMessagePiece& message = static_cast<cTorrentMessagePiece&> (msg);

	endian_swap(message.mPieceIndex);
	endian_swap(message.mBegin);

	u32 blockNumber = message.mBegin / BLOCK_LENGTH;

	const cRequestQueue::BlockRequest* pReq = peer.OutstandingRequestQueue().GetBlockRequest(message.mPieceIndex, blockNumber);
	if(!pReq)
	{
		// TODO ...
		if(0) //pTorrent->IsEndGame() || peer.CancelledLastBlock())
		{
			Printf("A canceled block seems to have arrived from %s. Piece %d Block %d\n", peer.Address().Ip().AsString(), message.mPieceIndex, blockNumber+1);

			// peer must have canceled the block while it was in transit, he may now be idle or downloading another 
			// block from another or the same piece
			return;
		}
		else
		{
			// some piece of shit clients (BitComet from what i can tell) seem to send bad PIECE messages where
			// mBegin will equal zero even when i requested a different block, get rid of them and log the problem

			char str[256];
			sprintf(str, "Bad PIECE. Arrived from %s. Piece %d Block %d (not in our outstanding list).", peer.Address().Ip().AsString(), message.mPieceIndex, blockNumber);
			peer.LogStandardMessage(0, str, cTorrentPacketLog::BAD_MESSAGE_CLOSE_CONNECTION, PTYPE_RECEIVE);

			Printf("%s\n", str);

			// you cannot do this!!!! there is a loop lower down using it!!!
			//pTorrent->DisconnectFromPeer(&peer);

			// a piece has arrived that we are not downloading, not cool
			//assert(0);

			return;
		}
	}
	
	const u8* pData = reinterpret_cast<const u8*> ((const u8*)(&message) + sizeof(cTorrentMessagePiece));
	s64 blockSize = message.mSize - (sizeof(cTorrentMessagePiece) - 4);	
	
	pTorrent->FileSet().ReadWritePiece(cFilePieceSet::WRITE, message.mPieceIndex, message.mBegin, blockSize, const_cast<u8*>(pData));

	peer.OnBlockDownloaded(pTorrent, message.mPieceIndex, message.mBegin, static_cast<u32>(blockSize), blockNumber);	
}// END MessageHandler_Piece



void MessageHandler_Cancel(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	cTorrentMessageCancel& message = static_cast<cTorrentMessageCancel&> (msg);

	endian_swap(message.mPieceIndex);
	endian_swap(message.mBegin);
	endian_swap(message.mLength);
}// END MessageHandler_Cancel



void MessageHandler_Port(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
//	endian_swap(message.mPort);
}// END MessageHandler_Port



