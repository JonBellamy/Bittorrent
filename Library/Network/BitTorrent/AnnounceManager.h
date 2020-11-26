// Jon Bellamy 09/01/2010
// Handles making the announce calls and passing on new peers to the torrent


#ifndef ANNOUNCE_MGR_H
#define ANNOUNCE_MGR_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <assert.h>
#include <vector>

#include "Network/SockAddr.h"
#include "Network/Url.h"
#include "Network/http/HttpClient.h"
#include "Network/BitTorrent/BitTorrentValues.h"


class cBitTorrent;


class cAnnounceManager
{
public:
    cAnnounceManager();
	~cAnnounceManager();

private:
	cAnnounceManager(const cAnnounceManager&);
	const cAnnounceManager& operator= (const cAnnounceManager& rhs);

public:

	void Start();
	void Stop();

	void Process();

	void RegisterTorrentForTrackerUpdates(cBitTorrent* pTorrent);
	void UnregisterTorrentForTrackerUpdates(cBitTorrent* pTorrent);
	bool AddAnnounceUrl(const net::cUrl& url);
	
	typedef enum
	{
		NONE=0,
		STARTED,
		STOPPED,
		COMPLETED
	}eAnnounceEvent;

	typedef enum
	{
		ANNOUNCE_SUCCESS = 0,
		ANNOUNCE_NEVER,
		ANNOUNCE_FAILED_TO_CONNECT,
		ANNOUNCE_FAIL_UNKNOWN,
	}ANNOUNCE_RESULT;

	typedef struct  
	{
		char mUrl[512];
		u32 mAnnounceIntervalMs;
		u32 mNextAnnounceInMs;
		u32 mNumberOfPeersFound;
		ANNOUNCE_RESULT mLastResult;
	}AnnounceDetails;

	u32 NumberOfAnnounceTargets(const cBitTorrent* pTorrent) const;
	AnnounceDetails GetAnnounceDetails(const cBitTorrent* pTorrent, u32 index) const;
	

private:	

	enum
	{
		ANNOUNCE_PERIOD = 1000 * 60 * 10,				// 10 minutes
		HTTP_BUFFER_SIZE = 1024 * 128
	};

	typedef struct  
	{
		u32 mAnnounceIntervalMs;
	}AnnounceResult;

	typedef struct  
	{
		u32 mId;
		cBitTorrent* mpParentTorrent;
		net::cUrl mUrl;
		u32 mLastAnnounceTime;
		ANNOUNCE_RESULT mLastResult;
		bool mForceAnnounceNow;
		AnnounceResult mAnnounceResult;
		std::vector<net::cSockAddr> mPeers;				// All the peers we have found via this tracker
	}AnnounceTarget;

	AnnounceTarget* GetAnnounceTarget(const cBitTorrent* pTorrent, const net::cUrl& url);
	AnnounceTarget* GetAnnounceTarget(u32 id);

	bool Announce(AnnounceTarget& target);
	static void HttpAnnounceCb(bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param);


	//////////////////////////////////////////////////////////////////////////
	// Member data

	bool mRunning;

	std::vector<net::cUrl> mUrlList;

	typedef std::vector<cBitTorrent*> RegdTorrentVector;
	typedef RegdTorrentVector::iterator RegdTorrentVectorIterator;
	RegdTorrentVector mRegisteredTorrents;

	typedef std::vector<AnnounceTarget> AnnounceTargetVector;
	typedef AnnounceTargetVector::iterator AnnounceTargetVectorIterator;
	AnnounceTargetVector mAnnounceList;

	u32 mNextAnnounceId;
};






#endif // ANNOUNCE_MGR_H
