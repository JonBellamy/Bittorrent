
#ifndef BT_CBS__H
#define BT_CBS__H


#include <string>
#include <vector>
#include "BitTorrentValues.h"


class cBitTorrent;


//////////////////////////////////////////////////////////////////////////
// Callback signatures 


typedef bool (*TorrentAddedCallback)	(TorrentHandle handle, void* pParam);
typedef bool (*TorrentRemovedCallback)	(TorrentHandle handle, void* pParam);
typedef void (*TorrentDownloadCompleteCallback)	(cBitTorrent* pTorrent, void* pParam);

typedef bool (*PeerConnectedCallback)	(TorrentHandle handle, u32 ip, u16 port, void* pParam);
typedef bool (*PeerDisconnectedCallback)	(TorrentHandle handle, u32 ip, u16 port, void* pParam);




#endif // BT_CBS__H