// Jon Bellamy 09/01/2010
// Handles making the announce calls and passing on new peers to the torrent



#include "AnnounceManager.h"

#include "General/Endianness.h"
#include "General/Rand.h"
#include "General/StringUtil.h"
#include "Network/BitTorrent/BitTorrent.h"
#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/BitTorrent/BEncoding/BencodedString.h"
#include "Network/BitTorrent/BEncoding/BencodedInt.h"
#include "Network/BitTorrent/BEncoding/BencodedList.h"



using namespace net;



cAnnounceManager::cAnnounceManager()
: mRunning(false)
, mNextAnnounceId(100)
{
}// END cAnnounceManager



cAnnounceManager::~cAnnounceManager()
{
	Stop();
}// END ~cAnnounceManager



void cAnnounceManager::Start()
{
	mRunning = true;
}// END Start



void cAnnounceManager::Stop()
{
	mRunning = false;
	mAnnounceList.clear();
}// END Stop



void cAnnounceManager::Process()
{
	if(mRunning == false ||
	   BitTorrentManager().Options().UseTrackers() == false)
	{
		return;
	}


	for(u32 i=0; i < mAnnounceList.size(); i++)
	{
		AnnounceTarget& target = mAnnounceList[i];

		assert(target.mpParentTorrent);

		if(target.mpParentTorrent->HttpClient().IsProcessingRequest() == false)
		{
			if(target.mpParentTorrent->Time() - target.mLastAnnounceTime >= ANNOUNCE_PERIOD || target.mForceAnnounceNow)
			{
				target.mForceAnnounceNow = false;				
				bool ret = Announce(target);
				target.mLastAnnounceTime = target.mpParentTorrent->Time();
				if(ret)
				{
					return;
				}
			}
		}
	}
}// END Process



bool cAnnounceManager::Announce(AnnounceTarget& target)
{
	assert(target.mpParentTorrent);
	assert(target.mpParentTorrent->HttpClient().IsProcessingRequest() == false);

	// file size
//	s64 totalSize = mpParentTorrent->MetaFile().TotalSizeOfContentInBytes();
//	char szTotalSize[32];
//	sprintf(szTotalSize, "%I64d", totalSize);

	// peerId
	std::string peerId = target.mpParentTorrent->PeerIdAsString();
	
	s64 bytesLeftToDownload = (target.mpParentTorrent->FileSet().NumberOfPieces() - target.mpParentTorrent->NumberOfPiecesDownloaded()) * target.mpParentTorrent->FileSet().PieceSize();
	char szBytesLeftToDownload[32];
	sprintf(szBytesLeftToDownload, "%I64d", bytesLeftToDownload);

	// listen port 
	char listenPort[32];
	sprintf(listenPort, "%u", BitTorrentManager().ListenPort());

	cUrl url = target.mUrl;
	url.AddParameter("info_hash", target.mpParentTorrent->InfoHashAsString());
	url.AddParameter("peer_id", peerId);
	url.AddParameter("left", szBytesLeftToDownload);
	url.AddParameter("compact", "1");
	url.AddParameter("no_peer_id", "0");
	url.AddParameter("numwant", "50");
	url.AddParameter("uploaded",  target.mpParentTorrent->BytesUploadedInThisSession());
	url.AddParameter("downloaded", target.mpParentTorrent->BytesDownloadedInThisSession());

	// Only send the started announce event 
	if(target.mLastAnnounceTime == ANNOUNCE_NEVER)
	{
		url.AddParameter("event", "started");
	}

#if USE_MSE
	// TODO : this needs to be read from  the options
	url.AddParameter("requirecrypto", "1");
	//url.AddParameter("supportcrypto", "1");
	url.AddParameter("cryptoport", DEFAULT_LISTEN_PORT_CRYPTO);
	url.AddParameter("port", listenPort);
#else
	url.AddParameter("port", listenPort);
#endif

	if(!target.mpParentTorrent->HttpClient().GET(url, STREAM_CONTENT_TO_BUFFER, NULL, NULL, HttpAnnounceCb, (void*)target.mId))
	{
		return false;
	}

	return true;
}// END Announce



void cAnnounceManager::HttpAnnounceCb(bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param)
{
	u32 id = reinterpret_cast<u32> (param);

	AnnounceTarget* pAnnounceTarget = BitTorrentManager().AnnounceManager().GetAnnounceTarget(id);
	if(pAnnounceTarget == NULL)
	{
		Printf("BitTorrent: ERROR: Announce target not found, redirection issue or torrent recently stopped / removed?");
		return;
	}

	assert(pAnnounceTarget->mpParentTorrent);

	if(success)
	{
		pAnnounceTarget->mLastResult = ANNOUNCE_SUCCESS;
	}
	else
	{
		pAnnounceTarget->mLastResult = ANNOUNCE_FAIL_UNKNOWN;
		return;
	}

	cBencodedDictionary dict;
	bool parseResult = dict.Parse(reinterpret_cast<const char*> (replyMessage.StreamedContentBuffer().Data()), replyMessage.StreamedContentBuffer().Size());
	
	if(!parseResult)
	{
		Printf("BitTorrent: Invalid response from tracker.\n");
		return;
	}
	
	
	Printf("BitTorrent: Got response dictionary.\n");

	// get the interval
	const cBencodedType* pBenType = dict.GetValue("interval");
	if(pBenType)
	{
		if(pBenType->Type() != cBencodedType::BEN_INT)
		{
			assert(0);
			return;
		}
		const cBencodedInt* benInterval = static_cast<const cBencodedInt *> (pBenType);

		pAnnounceTarget->mAnnounceResult.mAnnounceIntervalMs = static_cast<u32> (benInterval->Get() * 1000);
	}


	// parse the peer list 
	pBenType = dict.GetValue("peers");
	if(!pBenType)
	{
		//assert(0);

		// TODO : just remove this announce from our list?
		//pThis->mpParentTorrent->Stop();
		return;
	}

	std::vector<cPeerMetaData> addrList;

	if(pBenType->Type() == cBencodedType::BEN_LIST)
	{
		// dictionary peer model
		const cBencodedList* pBencodedPeersList = static_cast<const cBencodedList*> (pBenType);
		for(u32 i=0; i < pBencodedPeersList->NumElements(); i++)
		{
			const cBencodedDictionary* pBencodedPeerDict = static_cast<const cBencodedDictionary*> (pBencodedPeersList->GetElement(i));
			if(!pBencodedPeerDict || pBencodedPeerDict->Type() == cBencodedType::BEN_DICTIONARY)
			{
				assert(0);
				// TODO : just remove this announce from our list?
				//pThis->mpParentTorrent->Stop();
				return;
			}

			const cBencodedString* pBenPeerIdString = static_cast<const cBencodedString*> (pBencodedPeerDict->GetValue("peer id"));
			const cBencodedString* pBenPeerIpString = static_cast<const cBencodedString*> (pBencodedPeerDict->GetValue("peer id"));
			const cBencodedInt* pBenPeerPortInt = static_cast<const cBencodedInt*> (pBencodedPeerDict->GetValue("peer id"));
	
			if(!pBenPeerIdString || !pBenPeerIpString || !pBenPeerPortInt || 
				pBenPeerIdString->Type() != cBencodedType::BEN_STRING || pBenPeerIpString->Type() != cBencodedType::BEN_STRING || pBenPeerPortInt->Type() != cBencodedType::BEN_INT)
			{
				assert(0);
				// TODO : just remove this announce from our list?
				//pThis->mpParentTorrent->Stop();
				return;
			}	

			// TODO : finish off, need an example to see how ip is laid out...
			assert(0);
		}
	}
	else
	{
		// binary peer model
		if(!pBenType->Type() == cBencodedType::BEN_STRING)
		{
			assert(0);
			// TODO : just remove this announce from our list?
			//pThis->mpParentTorrent->Stop();
			return;
		}

		const cBencodedString* pBenPeersString = static_cast<const cBencodedString*> (pBenType);
		std::string strPeers = pBenPeersString->Get();

		
		//if(strPeers.size() % 6 != 0)
		//{
		//	assert(0);
		//	pThis->mpParentTorrent->Stop();
		//	return;
		//}

		u32 numPeers = static_cast<u32> (strPeers.size()) / 6;
		for(u32 i=0; i < numPeers; i ++)
		{
			u32 peerIndex = i * 6;

			u16 port;
			memcpy(&port, &strPeers[peerIndex+4], sizeof(u16));
			endian_swap(port);


			cSockAddr addr(cIpAddr(strPeers[peerIndex], strPeers[peerIndex+1], strPeers[peerIndex+2], strPeers[peerIndex+3]), port);

			if(addr.Ip() == cIpAddr(127,0,0,1))
			{
				continue;
			}

			addrList.push_back(cPeerMetaData(addr));

			// Store peers we have found for this tracker
			bool found = false;
			for(u32 j=0; j < pAnnounceTarget->mPeers.size(); j++)
			{
				if(pAnnounceTarget->mPeers[j] == addr)
				{
					found = true;
					break;
				}
			}
			if(found == false)
			{
				 pAnnounceTarget->mPeers.push_back(addr);
			}
		}
	}

	if(addrList.size() > 0)
	{
		pAnnounceTarget->mpParentTorrent->PresentPeers(addrList);
	}
}// END HttpAnnounceCb




void cAnnounceManager::RegisterTorrentForTrackerUpdates(cBitTorrent* pTorrent)
{
	mRegisteredTorrents.push_back(pTorrent);

	for(u32 i=0; i < mUrlList.size(); i++)
	{
		AnnounceTarget target;

		target.mId = mNextAnnounceId++;

		// This is a default value and will be overwritten by the 'interval' value returned by the tracker
		target.mAnnounceResult.mAnnounceIntervalMs = ANNOUNCE_PERIOD + Rand32(1000*60*10);

		target.mpParentTorrent = pTorrent;
		target.mUrl = mUrlList[i];
		target.mLastAnnounceTime = ANNOUNCE_NEVER;
		target.mForceAnnounceNow = true;
		mAnnounceList.push_back(target);
	}
}// END RegisterTorrentForTrackerUpdates



void cAnnounceManager::UnregisterTorrentForTrackerUpdates(cBitTorrent* pTorrent)
{
	for(RegdTorrentVectorIterator iter = mRegisteredTorrents.begin(); iter != mRegisteredTorrents.end(); iter++)
	{
		const cBitTorrent* pStoredTorrent = *iter;
		if(pTorrent == pStoredTorrent)
		{
			mRegisteredTorrents.erase(iter);
			break;
		}
	}
	

	bool found = true;
	while(found)
	{
		found = false;
		for(AnnounceTargetVectorIterator iter = mAnnounceList.begin(); iter != mAnnounceList.end(); iter++)
		{
			const AnnounceTarget& target = *iter;
			if(target.mpParentTorrent == pTorrent)
			{
				mAnnounceList.erase(iter);
				found = true;
				break;
			}
		}
	}
}// END UnregisterTorrentForTrackerUpdates



bool cAnnounceManager::AddAnnounceUrl(const cUrl& url)
{
	// Check for dupes
	for(u32 i=0; i < mUrlList.size(); i++)
	{
		if(StrCmpCaseInsensitive(mUrlList[i].AsString(), url.AsString()) == true)
		{
			return false;
		}
	}

	mUrlList.push_back(url);


	// Subscribe all registered torrents to the new tracker
	for(RegdTorrentVectorIterator iter = mRegisteredTorrents.begin(); iter != mRegisteredTorrents.end(); iter++)
	{
		cBitTorrent* pTorrent = *iter;

		AnnounceTarget target;

		target.mId = mNextAnnounceId++;

		// This is a default value and will be overwritten by the 'interval' value returned by the tracker
		target.mAnnounceResult.mAnnounceIntervalMs = ANNOUNCE_PERIOD + Rand32(1000*60*10);

		target.mpParentTorrent = pTorrent;
		target.mUrl = url;
		target.mLastAnnounceTime = ANNOUNCE_NEVER;
		target.mForceAnnounceNow = true;
		mAnnounceList.push_back(target);
	}

	return true;
}// END AddAnnounceUrl



cAnnounceManager::AnnounceDetails cAnnounceManager::GetAnnounceDetails(const cBitTorrent* pTorrent, u32 index) const
{
	AnnounceDetails details;
	memset(&details, 0, sizeof(AnnounceDetails));

	if(index >= NumberOfAnnounceTargets(pTorrent))
	{
		assert(0);
		return details;
	}

	u32 count = 0;
	AnnounceTarget target;
	for(u32 i=0; i < mAnnounceList.size(); i++)
	{
		if(mAnnounceList[i].mpParentTorrent == pTorrent)
		{
			if(count == index)
			{
				target = mAnnounceList[i];
				break;
			}

			count++;
		}
	}
	strcpy_s(details.mUrl, sizeof(details.mUrl), target.mUrl.AsString().c_str());
	details.mNextAnnounceInMs = target.mAnnounceResult.mAnnounceIntervalMs - (target.mpParentTorrent->Time() - target.mLastAnnounceTime);
	details.mAnnounceIntervalMs = target.mAnnounceResult.mAnnounceIntervalMs;
	details.mLastResult = target.mLastResult;
	details.mNumberOfPeersFound = target.mPeers.size();
	return details;
}// END GetAnnounceDetails



u32 cAnnounceManager::NumberOfAnnounceTargets(const cBitTorrent* pTorrent) const
{ 
	u32 count = 0;
	for(u32 i=0; i < mAnnounceList.size(); i++)
	{
		if(mAnnounceList[i].mpParentTorrent == pTorrent)
		{
			count++;
		}
	}
	return count; 
}// END NumberOfAnnounceTargets



cAnnounceManager::AnnounceTarget* cAnnounceManager::GetAnnounceTarget(const cBitTorrent* pTorrent, const cUrl& url)
{
	for(u32 i=0; i < mAnnounceList.size(); i++)
	{
		// TODO : We are only checking hostname here, it should be more thorough
		if( pTorrent == mAnnounceList[i].mpParentTorrent &&
			mAnnounceList[i].mUrl.HostName() == url.HostName())
		{
			return &(mAnnounceList[i]);
		}
	}
	return NULL;
}// END GetAnnounceTarget


cAnnounceManager::AnnounceTarget* cAnnounceManager::GetAnnounceTarget(u32 id)
{
	for(u32 i=0; i < mAnnounceList.size(); i++)
	{
		if(mAnnounceList[i].mId == id)
		{
			return &mAnnounceList[i];
		}
	}
	return NULL;
}// END GetAnnounceTarget