// Jon Bellamy 08/12/2009
// The DhtResourceInfoManager holds records about resources. Each record contains peer information.
// If a peer announces they are participating in the distribution of a resource, they are added.
// Some peers are added if they know of other peers who have the resource and we want to (at some point)
// announce to them. The fact that a peer has an entry for a resource does NOT mean they have it.


#include "ResourceContactInfo.h"




using namespace net;


static cDhtResourceInfoManager gResInfoMgr;
extern cDhtResourceInfoManager& DhtResourceInfoManager()
{
	return gResInfoMgr;
}// END DhtResourceInfoManager



//////////////////////////////////////////////////////////////////////////
// cDhtResourceInfoManager



cDhtResourceInfoManager::cDhtResourceInfoManager()
{
}// END cDhtResourceInfoManager



cDhtResourceInfoManager::~cDhtResourceInfoManager()
{
	while(mResources.size() > 0)
	{
		cDhtResourceContactInfo* pResInfo = mResources.back();
		delete pResInfo;
		mResources.pop_back();
	}
}// END ~cDhtResourceInfoManager



cDhtResourceContactInfo* cDhtResourceInfoManager::CreateRecordForResource(const cDhtResourceId& resource)
{
	if(HaveRecordForResource(resource))
	{
		return NULL;
	}
	cDhtResourceContactInfo* pResource = new cDhtResourceContactInfo(resource);
	mResources.push_back(pResource);
	return pResource;
}// END CreateRecordForResource



bool cDhtResourceInfoManager::DeleteRecordForResource(const cDhtResourceId& resource)
{
	for(ResourceListIterator iter = mResources.begin(); iter != mResources.end(); iter++)
	{
		cDhtResourceContactInfo* pResInfo = *iter;
		assert(pResInfo);
		
		if(pResInfo->Resource() == resource)
		{
			delete pResInfo;
			mResources.erase(iter);
			return true;
		}
	}
	
	return false;
}// END DeleteRecordForResource



bool cDhtResourceInfoManager::HaveRecordForResource(const cDhtResourceId& resource)
{
	return GetRecordForResource(resource) != NULL;
}// END HaveRecordForResource



cDhtResourceContactInfo* cDhtResourceInfoManager::GetRecordForResource(const cDhtResourceId& resource)
{
	for(ResourceListIterator iter = mResources.begin(); iter != mResources.end(); iter++)
	{
		cDhtResourceContactInfo* pResInfo = *iter;
		assert(pResInfo);

		if(pResInfo->Resource() == resource)
		{
			return pResInfo;
		}
	}
	return NULL;
}// END GetRecordForResource



// Someone is looking for a resource, keep a record in case they later announce that they have it
void cDhtResourceInfoManager::OnGetPeersReceived(const cDhtNode& node, const cDhtResourceId& resource, const std::string& tokenWeGenerated)
{
	cDhtResourceContactInfo* pResource = GetRecordForResource(resource);
	if(pResource == NULL)
	{
		pResource = CreateRecordForResource(resource);
		assert(pResource);		
	}
	pResource->AddPeer(node, tokenWeGenerated, "");
}// END OnGetPeersReceived



// We sent out a GetPeers query & someone replied. If this is called to add them then we want to later announce to them.
void cDhtResourceInfoManager::OnGetPeersResponse(const cDhtNode& node, const cDhtResourceId& resource, const std::string& tokenTheyGenerated)
{
	cDhtResourceContactInfo* pResource = GetRecordForResource(resource);
	if(pResource == NULL)
	{
		pResource = CreateRecordForResource(resource);
		assert(pResource);		
	}
	pResource->AddPeer(node, "", tokenTheyGenerated);
}// END OnGetPeersResponse



cPeerWithResource* cDhtResourceInfoManager::GetPeerInfo(const cDhtNode& node, const cDhtResourceId& resource)
{
	cDhtResourceContactInfo* pResRecord = GetRecordForResource(resource);
	if(!pResRecord)
	{
		return NULL;
	}
	return pResRecord->GetPeer(node);
}// END GetPeerInfo



void cDhtResourceInfoManager::AllPeersForResource(const cDhtResourceId& resource, std::vector<cPeerWithResource>* pVecOut, bool peerMustHaveAnnounced)
{
	pVecOut->clear();
	cDhtResourceContactInfo* pResRecord = GetRecordForResource(resource);
	if(!pResRecord)
	{
		return;
	}

	const std::deque<cPeerWithResource>& allPeers = pResRecord->GetPeerList();
	for(u32 i=0; i < allPeers.size(); i++)
	{
		if(peerMustHaveAnnounced)
		{
			if(allPeers[i].mbHasAnnouncedToUs)
			{
				pVecOut->push_back(allPeers[i]);
			}
		}
		else
		{
			pVecOut->push_back(allPeers[i]);
		}
	}
}// END AllPeersForResource




//////////////////////////////////////////////////////////////////////////
// cDhtResourceContactInfo


cDhtResourceContactInfo::cDhtResourceContactInfo(const cDhtResourceId& infoHash)
: mInfoHash(infoHash)
{
}// END cDhtResourceContactInfo



bool cDhtResourceContactInfo::AddPeer(const cDhtNode& node, const std::string& internalToken, const std::string& externalToken)
{
	if(GetPeer(node) != NULL)
	{
		return false;
	}

	cPeerWithResource res(node, internalToken, externalToken);
	mPeersWithResource.push_back(res);
	return true;
}// END AddPeer



cPeerWithResource* cDhtResourceContactInfo::GetPeer(const cDhtNode& node)
{
	for(u32 i=0; i < mPeersWithResource.size(); i++)
	{
		cPeerWithResource& peerInfo = mPeersWithResource[i];
		if (peerInfo.mNode == node)
		{
			return &peerInfo;
		}
	}
	return NULL;
}// END GetPeer