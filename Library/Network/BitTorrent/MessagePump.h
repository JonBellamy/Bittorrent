// Jon Bellamy 22/02/2009


#ifndef BITTORRENT_MESSAGE_PUMP_H
#define BITTORRENT_MESSAGE_PUMP_H

#if USE_PCH
#include "stdafx.h"
#endif


#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/BitTorrent/BitTorrentMessages.h"
#include "Network/BitTorrent/BitTorrentPeer.h"

class cBitTorrent;

// function ptr for message handlers
typedef void (*PtrMessageHandler) (cTorrentPeer& /*fromPeerId*/, cBitTorrent* /*pTorrent*/, cTorrentMessage& /*msg*/);


class cNetMessagePump
{
public:

	cNetMessagePump();
	~cNetMessagePump();


	void RegisterMessageHandler(TorrentMessageId msgType, PtrMessageHandler msgHandler);
	void DispatchNetMessage(cTorrentPeer& peer, cBitTorrent* pTorrent, cTorrentMessage& msg);


private:

	PtrMessageHandler GetMessageHandler(TorrentMessageId msgType);


	PtrMessageHandler mHandlers[NUM_TORRENT_MESSAGE_TYPES];
};



// Singleton
inline cNetMessagePump& NetMessagePump()
{
	static cNetMessagePump pump;
	return pump;
}// END NetMessagePump




#endif // BITTORRENT_MESSAGE_PUMP_H
