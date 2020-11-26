#ifndef BITTORRENT_VALUES_H
#define BITTORRENT_VALUES_H



#define USE_MSE 1
#define USE_DHT 1



typedef s32 TorrentHandle;

enum
{
	INVALID_TORRENT_HANDLE = 0xFFFFFFFF
};


enum
{
	SOCKET_SEND_BUFFER_SIZE = 1024 * 128,
	SOCKET_RECV_BUFFER_SIZE = 1024 * 512,

	PEER_ID_LENGTH = 20,
	INFO_HASH_LENGTH = 20,

	BLOCK_LENGTH = 1024 * 16,				// we download blocks, a number of blocks make up a piece

	DEFAULT_LISTEN_PORT = 47477,

	DEFAULT_LISTEN_PORT_CRYPTO = 50477,

	HANDSHAKE_LENGTH = 68,
	HANDSHAKE_TIMEOUT_MS = 30000,			// x seconds to get the handshake

	GLOBAL_MAX_PEER_CONNECTIONS = 200,		// uTorrent defaults
	MAX_PEER_CONNECTIONS_PER_TORRENT = 50,
	NUM_CONCURRENT_TORRENTS_FOR_QUEUED_TO_START = 4,

	MAX_HALF_OPEN_CONNECTIONS_PRE_WIN7 = 10,
	MAX_HALF_OPEN_CONNECTIONS = 100,

	DEFAULT_UPLOAD_LIMIT = 1024 * 512,

	DHT_NUMBER_OF_NODES_PER_GET_PEER_SEARCH = 32,

	MAX_FLAGS = 32
};


// Bit Torrent Flags
typedef enum
{
	BT_COMPLETE = 1 << 0,							// set when the torrent finishes downloading

	NUM_EVENT_FLAGS
}BitTorrentEventFlag;



#define TORRENT_STATE_STARTED									"started"
#define TORRENT_STATE_STOPPED									"stopped"
#define TORRENT_STATE_QUEUED									"queued"


#define XML_FILENAME											"BtmState.xml"
#define XML_ROOT												"App_State"
#define XML_OPTIONS_ROOT										"App_Options"
#define XML_TORRENTS_ROOT										"Torrents"

#define XML_OPTION_VALUE										"value"
#define XML_APP_OPTION_ALLOW_UNSECURE_CONNECTIONS				"AllConnectionsMustBeEncrypted"
#define XML_APP_OPTION_DHT_ENABLED								"DhtEnabled"
#define XML_APP_OPTION_TRACKERS_ENABLED							"TrackersEnabled"
#define XML_APP_OPTION_CHECK_BUILD								"CheckBuild"
#define XML_APP_OPTION_LISTEN_PORT								"ListenPort"
#define XML_APP_OPTION_STOP_ON_COMPLETION						"StopAllTorrentsOnCompletion"
#define XML_APP_OPTION_MAX_UPLOAD_RATE							"MaxUploadRate"


#define XML_ITEM_ID "Torrent"
#define XML_ITEM_NAME "filename"
#define XML_ITEM_ROOT "root_folder"
#define XML_ITEM_STATE "state"
#define XML_ITEM_PARTIAL_PIECES "partial_pieces"
#define XML_ITEM_PIECE "piece"
#define XML_ITEM_PIECE_NUMBER "piece_number"
#define XML_ITEM_PIECE_NUMBER_OF_BLOCKS "number_of_blocks"
#define XML_ITEM_PIECE_SIZE "piece_size"
#define XML_ITEM_PIECE_BITFIELD "bitfield"

#endif // BITTORRENT_VALUES_H


