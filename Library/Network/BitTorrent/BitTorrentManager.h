// Jon Bellamy 10/01/2010


#ifndef BITTORRENT_MAN_H
#define BITTORRENT_MAN_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <vector>
#include <string>

#include "Network/BitTorrent/BitTorrentValues.h"
#include "Network/BitTorrent/BitTorrentCallbacks.h"
#include "Network/BitTorrent/AnnounceManager.h"
#include "Network/BitTorrent/Mse/MessageStreamEncryption.h"
#include "Network/BitTorrent/BitTorrent.h"
#include "Network/BitTorrent/TorrentOptions.h"
#include "Network/SockAddr.h"
#include "dht/DhtTaskManager.h"
#include "General/Timer.h"
#include "General/CallbackSet.h"




class cBitTorrentManager
{
public:
    cBitTorrentManager();
	~cBitTorrentManager();

private:
	cBitTorrentManager(const cBitTorrentManager&);
	const cBitTorrentManager& operator= (const cBitTorrentManager& rhs);


public:
	
	void Init(const char* appDataFolder);
	void DeInit();

	cDhtTaskManager& DhtTaskManager() { return mDhtTaskManager; }

	// returns the torrents uid
	TorrentHandle AddTorrent(const char* szTorrentFile, const char* rootDir);
	bool RemoveTorrent(TorrentHandle handle);
	void RemoveAllTorrents();

	void StartTorrent(TorrentHandle handle);
	void StopTorrent(TorrentHandle handle);
	void PauseTorrent(TorrentHandle handle);

	void Process();
	void ProcessIncomingPeerConnections();
	void ProcessNewConnections();

	u32 GetAllTorrentHandles(TorrentHandle* pHandleArray, u32 arraySize);
	TorrentHandle GetHandle(cBitTorrent* pTorrent);

	cBitTorrent* GetTorrent(TorrentHandle handle);	
	cBitTorrent* GetTorrent(const u8* infoHash);
	const cBitTorrent* GetTorrent(const u8* infoHash) const;
	const u8* GetTorrentInfoHashFromReq2Hash(u8* req2Hash);

	u32 DownloadRate() const;
	u32 UploadRate() const;

	u32 TimeTorrentManagerRunning() const { return mTimer.ElapsedMs(); }

	u32 MaxUploadRate() const { return mOptions.MaxUploadRate(); }

	bool TorrentConnectionsMustBeEncrypted(u8* infohash) const;

	void OnOptionsUdated();
	sTorrentOptions& Options() { return mOptions; }
	const sTorrentOptions& Options() const { return mOptions; }

	bool SaveState();
	bool LoadState();

	void SetIncomingConnectionsListener(u16 port);
	u16 ListenPort() const { return mListenPort; }

	// Used to limit peer connections across all torrent
	u32 GlobalNumberOfPeerConnections() const;
	bool CanMakePeerConnection(cBitTorrent* pTorrent, bool existingConnection) const;
	void OnPeerConnection(cBitTorrent* pTorrent, const net::cSockAddr& addr);
	void OnPeerDiconnection(cBitTorrent* pTorrent, const net::cSockAddr& addr);

	const std::string& AppDataFolder() const { return mAppDataFolder; }
	const std::string& PeerLogFolder() const { return mPeerLogFolder; }
	const std::string& DhtLogFolder() const { return mDhtLogFolder; }

	void AddTorrentAddedCallback(TorrentAddedCallback cb, void* pParam) { mTorrentAddedHandlers.Add(NULL, cb, pParam); }
	bool RemoveTorrentAddedCallback(TorrentAddedCallback cb, void* pParam) { return mTorrentAddedHandlers.Remove(NULL, cb, pParam); }

	void AddTorrentRemovedCallback(TorrentAddedCallback cb, void* pParam) { mTorrentRemovedHandlers.Add(NULL, cb, pParam); }
	bool RemoveTorrentRemovedCallback(TorrentAddedCallback cb, void* pParam) { return mTorrentRemovedHandlers.Remove(NULL, cb, pParam); }

	void AddPeerConnectedCallback(PeerConnectedCallback cb, void* pParam) { mPeerConnectedHandlers.Add(NULL, cb, pParam); }
	bool RemovePeerConnectedCallback(PeerConnectedCallback cb, void* pParam) { return mPeerConnectedHandlers.Remove(NULL, cb, pParam); }

	void AddPeerDisconnectedCallback(PeerDisconnectedCallback cb, void* pParam) { mPeerDisconnectedHandlers.Add(NULL, cb, pParam); }
	bool RemovePeerDisconnectedCallback(PeerDisconnectedCallback cb, void* pParam) { return mPeerDisconnectedHandlers.Remove(NULL, cb, pParam); }

	cAnnounceManager& AnnounceManager();

	typedef enum
	{
		STOPPED = 0,
		INITIALIZING,
		RUNNING
	}BtmState;
	BtmState State() const { return mState; }

private:

	void SaveTrackerCache();
	void LoadTrackerCache();

	u32 NumTorrentsQueued() const;
	u32 NumTorrentsDownloading() const;	

	void ProcessQueuedTorrents();

	static void TorrentDownloadCompleteHandler(cBitTorrent* pTorrent, void* pParam);

	enum
	{
		MAX_DHT_GETPEER_SEARCH_HISTORY_SIZE = 2048								// Don't let the dht tasks get out of control, each node saved costs 6 bytes
	};

	static void DhtFindPeersCb (const cDhtTask* pTask, void* param);

	class ManagedTorrent
	{
	public:
		ManagedTorrent()
		: mTorrent(NULL)
#if USE_DHT
		, mpDhtGetPeersTask(NULL)
#endif
		{
		}

		TorrentHandle mHandle;
		cBitTorrent* mTorrent;

#if USE_DHT
		const cDhtGetPeersTask* mpDhtGetPeersTask;
#endif

		// Used to identify which torrent an incoming encrypted connection is for
		u8 mReq2InfoHash[20];		// HASH('req2', SKEY)
	};

	cAnnounceManager mAnnounceManager;

	cDhtTaskManager mDhtTaskManager;
	cTimer mTimer;
	sTorrentOptions mOptions;

	net::cTcpSocket mListener;
	net::cTcpSocket mCryptoListener;
	u16 mListenPort;

	typedef std::vector<ManagedTorrent> TorrentsVector;
	typedef TorrentsVector::iterator TorrentsVectorIterator;
	typedef TorrentsVector::const_iterator TorrentsVectorConstIterator;
	TorrentsVector mTorrents;

	mutable float mDownloadBandwidthPeak;
	mutable float mUploadBandwidthPeak;

	class NewTorrentConnection
	{
	private:
		NewTorrentConnection();
	public:
		NewTorrentConnection(u32 time, const cEncryptedTorrentConnection& s)
		: mConnectionTime(time)
		, mSocket(s)
		{
		}

		const NewTorrentConnection& operator= (const NewTorrentConnection& rhs)
		{
			mConnectionTime = rhs.mConnectionTime;
			mSocket = rhs.mSocket; 
			return *this;
		}

		u32 mConnectionTime;
		cEncryptedTorrentConnection mSocket;
	};

	BtmState mState;

	std::string mAppDataFolder;
	std::string mPeerLogFolder;
	std::string mDhtLogFolder;

	// new connections are held here until we can peek their handshake and tell which torrent they want
	typedef std::vector<NewTorrentConnection> NewConnectionVector;
	typedef NewConnectionVector::iterator NewConnectionIterator;
	typedef NewConnectionVector::const_iterator NewConnectionConstIterator;
	NewConnectionVector mNewConnections;

	TorrentHandle mNextTorrentId;

	// Events
	typedef cCallbackSet<NullFilterSig, TorrentAddedCallback> TorrentAddedCallbackSet;
	TorrentAddedCallbackSet mTorrentAddedHandlers;

	typedef cCallbackSet<NullFilterSig, TorrentRemovedCallback> TorrentRemovedCallbackSet;
	TorrentRemovedCallbackSet mTorrentRemovedHandlers;

	typedef cCallbackSet<NullFilterSig, PeerConnectedCallback> PeerConnectedCallbackSet;
	PeerConnectedCallbackSet mPeerConnectedHandlers;

	typedef cCallbackSet<NullFilterSig, PeerDisconnectedCallback> PeerDisconnectedCallbackSet;
	PeerDisconnectedCallbackSet mPeerDisconnectedHandlers;
};


extern cBitTorrentManager& BitTorrentManager();






#endif // BITTORRENT_MAN_H
