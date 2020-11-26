// Jon Bellamy 08/12/2009
// The DhtResourceInfoManager holds records about resources. Each record contains peer information.
// If a peer announces they are participating in the distribution of a resource, they are added.
// Some peers are added if they know of other peers who have the resource and we want to (at some point)
// announce to them. The fact that a peer has an entry for a resource does NOT mean they have it.


#ifndef DHT_RES_CONTACT_H_
#define DHT_RES_CONTACT_H_

#if USE_PCH
#include "stdafx.h"
#endif

#include <string>
#include <deque>
#include <vector>
#include <assert.h>
#include "Network/BitTorrent/dht/Node.h"



class cPeerWithResource
{
public:
	cPeerWithResource() { assert(0); }

	cPeerWithResource::cPeerWithResource(const cDhtNode& node, const std::string& internalToken, const std::string& externalToken) 
		: mNode(node)
		, mInternalToken(internalToken)
		, mExternalToken(externalToken)
		, mAnnouncedPort(0)
		, mbHasAnnouncedToUs(false)			// if this is false, mAnnouncedPort is invalid
	{
	}

	const cPeerWithResource& operator= (const cPeerWithResource& rhs)
	{
		mNode = rhs.mNode;
		mInternalToken = rhs.mInternalToken;
		mExternalToken = rhs.mExternalToken;
		mAnnouncedPort = rhs.mAnnouncedPort;
		mbHasAnnouncedToUs = rhs.mbHasAnnouncedToUs;
		return *this;
	}

private:	
	bool operator== (const cPeerWithResource& rhs) const;

public:
	cDhtNode mNode;
	std::string mInternalToken;			// we generated this & sent it to them in response to a GetPeers from them
	std::string mExternalToken;			// they generated this & sent it to us after we sent them a GetPeers
	
	u16 mAnnouncedPort;
	bool mbHasAnnouncedToUs;
};



class cDhtResourceContactInfo
{
public:
	cDhtResourceContactInfo(const cDhtResourceId& infoHash);	
	bool operator== (const cDhtResourceContactInfo& rhs) const { return mInfoHash == rhs.mInfoHash; }

	const cDhtResourceId& Resource() const { return mInfoHash; }

private:
	cDhtResourceContactInfo(const cDhtResourceContactInfo& infoHash);
	const cDhtResourceContactInfo& operator= (const cDhtResourceContactInfo& rhs);


public:

	bool AddPeer(const cDhtNode& node, const std::string& internalToken, const std::string& externalToken);

	const std::deque<cPeerWithResource>& GetPeerList() const { return mPeersWithResource; }

	cPeerWithResource* GetPeer(const cDhtNode& node);

private:

	cDhtResourceId mInfoHash;	
	std::deque<cPeerWithResource> mPeersWithResource;
};



class cDhtResourceInfoManager
{
public:
	cDhtResourceInfoManager();
	~cDhtResourceInfoManager();

	cDhtResourceContactInfo* CreateRecordForResource(const cDhtResourceId& resource);
	bool DeleteRecordForResource(const cDhtResourceId& resource);

	bool HaveRecordForResource(const cDhtResourceId& resource);

	// Someone is looking for a resource, keep a record in case they later announce that they have it
	void OnGetPeersReceived(const cDhtNode& node, const cDhtResourceId& resource, const std::string& tokenWeGenerated);

	// We sent out a GetPeers query & someone replied. If this is called to add them then we want to later announce to them.
	void OnGetPeersResponse(const cDhtNode& node, const cDhtResourceId& resource, const std::string& tokenTheyGenerated);

	// Someone is announcing they are participating in a resource, token must match the one we generated for them
	//void OnAnnounceReceived(const cDhtNode& node, const cDhtResourceId& resource, const std::string& token);

	// Announce to all involved that we are now participating in this resource
	//void AnnounceToPeers(const cDhtResourceId& resource);

	// TODO
	// LIST PeerListForResource(const cDhtResourceId& resource);

	cPeerWithResource* GetPeerInfo(const cDhtNode& node, const cDhtResourceId& resource);

	void AllPeersForResource(const cDhtResourceId& resource, std::vector<cPeerWithResource>* pVecOut, bool peerMustHaveAnnounced);

private:	

	typedef std::deque<cDhtResourceContactInfo*> ResourceList;
	typedef ResourceList::iterator ResourceListIterator;
	typedef ResourceList::const_iterator ResourceListConstIterator;
	

	cDhtResourceContactInfo* GetRecordForResource(const cDhtResourceId& resource);


	ResourceList mResources;
};


extern cDhtResourceInfoManager& DhtResourceInfoManager();






#endif // DHT_RES_CONTACT_H_