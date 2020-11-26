// Jon Bellamy 22/02/2009


#include "MessagePump.h"

#include "Network/BitTorrent/BitTorrent.h"




cNetMessagePump::cNetMessagePump()
{
	RegisterMessageHandler(TM_CHOKE, MessageHandler_Choke);
	RegisterMessageHandler(TM_UNCHOKE, MessageHandler_Unchoke);
	RegisterMessageHandler(TM_INTERESTED, MessageHandler_Interested);
	RegisterMessageHandler(TM_NOT_INTERESTED, MessageHandler_NotInterested);
	RegisterMessageHandler(TM_HAVE, MessageHandler_Have);
	RegisterMessageHandler(TM_BITFIELD, MessageHandler_Bitfield);
	RegisterMessageHandler(TM_REQUEST,MessageHandler_Request);
	RegisterMessageHandler(TM_PIECE, MessageHandler_Piece);
	RegisterMessageHandler(TM_CANCEL, MessageHandler_Cancel);
	RegisterMessageHandler(TM_PORT, MessageHandler_Port);
}// END cNetMessagePump



cNetMessagePump::~cNetMessagePump()
{
}// END ~cNetMessagePump




void cNetMessagePump::RegisterMessageHandler(TorrentMessageId msgType, PtrMessageHandler msgHandler)
{
	mHandlers[msgType] = msgHandler;
}// END RegisterMessageHandler



void cNetMessagePump::DispatchNetMessage(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg)
{
	TorrentMessageId msgType = msg.Type();
	PtrMessageHandler handler = GetMessageHandler(msgType);
	handler(peer, pTorrent, msg);
}// END DispatchMessages



PtrMessageHandler cNetMessagePump::GetMessageHandler(TorrentMessageId msgType)
{
	return mHandlers[msgType];
}// END GetMenuHandler
