// Jon Bellamy 18/11/2009


#ifndef DHT_ROUTING_TABLE_H_
#define DHT_ROUTING_TABLE_H_


#if USE_PCH
#include "stdafx.h"
#endif


#include <list>
#include <vector>

#include "Network/SockAddr.h"
#include "Network/BitTorrent/dht/Node.h"


// TODO : HUGE PROBLEM WITH THIS. We are returning ptr's to nodes from the routing table which is an stl list which WILL expand and invalidate the ptr's. We need to 
// just use the vector of nodes below and remove this!!!!!!!!!!!!!!!
// STL list may be saving our ass as it invalidates erased portions but still ...
typedef std::vector<cDhtNode*> DhtNodePtrVector;
typedef DhtNodePtrVector::iterator DhtNodePtrVectorIterator;
typedef DhtNodePtrVector::const_iterator DhtNodePtrVectorConstIterator;


typedef std::vector<cDhtNode> DhtNodeVector;
typedef DhtNodeVector::iterator DhtNodeVectorIterator;
typedef DhtNodeVector::const_iterator DhtNodeVectorConstIterator;



// TODO : we need to keep the cached lists of close nodes etc, it building these lists LOTS

class cDhtRoutingTable
{
public:
    cDhtRoutingTable();
	~cDhtRoutingTable();

private:
	cDhtRoutingTable(const cDhtRoutingTable& rhs);
	const cDhtRoutingTable& operator= (const cDhtRoutingTable& rhs);
	bool operator== (const cDhtRoutingTable& rhs);

	
public:

	void Load();
	void Save() const;

	void Clear();

	void Process();

	// all nodes that are discovered are passed to this function in case the routing table wants to add it
	bool PresentNode(const cDhtNode& node);

	u32 NumberOfCloseNodes() const;
	u32 NumberOfFarNodes() const;

	bool HasEnoughCloseNodes() const;

	void FurthestCloseNodeDistance(cDhtNode* pNodeOut, u32* distanceOut);

	bool InsertNode(const cDhtNode& node);
	bool RemoveNode(const cDhtNodeId& nodeId);
	bool RemoveNode(const cDhtNode& node);

	// HUGE TODO: This is almost certainly causing crashes, node ptr's need to go away
	const cDhtNode* Node(const cDhtNodeId& nodeId);
	cDhtNode* Node(const net::cSockAddr& sockAddr);
	bool Node(u32 nodeIndex, cDhtNode* nodeOut);


	void AllNodesWithInvalidId(DhtNodePtrVector* pOutVector);


	// returns an ordered vector with the required nodes
	void CloseNodes(DhtNodePtrVector* pOutVector);
	void FarNodes(DhtNodePtrVector* pOutVector);

	// When a node wants to find peers for a torrent, it uses the distance metric to compare the infohash of the torrent with the IDs of the nodes in its own routing table. 
	// It then contacts the nodes it knows about with IDs closest to the infohash and asks them for the contact information of peers currently downloading the torrent.
	void ClosestNodeToInfoHash(const cDhtResourceId& infoHash, u32 maxNodesToReturn, DhtNodePtrVector* pOutVector);

	u32 Size() const { return static_cast<u32> (mRoutingTable.size()); }


	void DebugPrint();

private:

	// returns a vector containing the current routing table sorted by distance to our local node.
	void RoutingTableNodesSortedByDistanceToNode(const cDhtNodeId& node, DhtNodePtrVector* pOutVector);

	friend class cDht;

	enum
	{
		NUM_CLOSE_NODES_TO_KEEP = 512,
		NUM_FAR_NODES_TO_KEEP = 128,

		// node distance runs from 0 - 5120
		CLOSE_NODE_DISTANCE = 2048
	};

	//typedef u32 NodeDistance;
	typedef std::list<cDhtNode> RoutingTableList;
	typedef RoutingTableList::iterator RoutingTableIterator;
	typedef RoutingTableList::const_iterator RoutingTableConstIterator;


	RoutingTableIterator NodeIter(const cDhtNodeId& nodeId);



	RoutingTableList mRoutingTable;
};






#endif // DHT_ROUTING_TABLE_H_