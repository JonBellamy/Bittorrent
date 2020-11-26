// Jon Bellamy 18/11/2009


#include "RoutingTable.h"

#include <assert.h>
#include <algorithm>


#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/BitTorrent/dht/Dht.h"
#include "Network/BitTorrent/dht/SortObjects.h"
#include "File/file.h"


using namespace net;


cDhtRoutingTable::cDhtRoutingTable()
{
}// END cDhtRoutingTable


cDhtRoutingTable::~cDhtRoutingTable()
{
}


void cDhtRoutingTable::Load()
{
	cFile file;

	std::string fn = BitTorrentManager().AppDataFolder() + "\\RoutingTable.bin";
	bool ret = file.Open(cFile::READ, fn.c_str(), true);

	if(ret && 
	   file.Size() > 0 && 
	   ((file.Size() % cDhtNode::COMPACT_NODE_SIZE)==0))
	{
		const u8* compactData = file.CachedFileData();
		assert(compactData);
		if(compactData)
		{
			u32 numNodes = file.Size() / cDhtNode::COMPACT_NODE_SIZE;
			for(u32 i=0; i < numNodes; i++)
			{
				cDhtNode node;
				node.FromCompactRepresentation(compactData);
				compactData += cDhtNode::COMPACT_NODE_SIZE;

				PresentNode(node);
				//InsertNode(node);
			}
		}
	}
}// END Load



void cDhtRoutingTable::Save() const
{
	if(mRoutingTable.empty() == false)
	{
		std::string fn = BitTorrentManager().AppDataFolder() + "\\RoutingTable.bin";
		cFile file(cFile::WRITE, fn.c_str());
		for(RoutingTableConstIterator iter = mRoutingTable.begin(); iter != mRoutingTable.end(); iter++)
		{
			const cDhtNode& node = *iter;

			if(node.Id().IsValid())
			{
				std::string compact = node.ToCompactRepresentation();
				file.Write(cFile::FILE_POS_CURRENT, cDhtNode::COMPACT_NODE_SIZE, reinterpret_cast<const u8*> (compact.c_str()));
			}
		}
		file.Close();
	}
}// END Save



void cDhtRoutingTable::Clear()
{
	if(mRoutingTable.empty() == false)
	{
		mRoutingTable.clear();
	}
}// END Clear



void cDhtRoutingTable::Process()
{
}// END Process



// all nodes that are discovered are passed to this function in case the routing table wants to add it
bool cDhtRoutingTable::PresentNode(const cDhtNode& node)
{
	if(Node(node.Id()))
	{
		return false;
	}

	// don't add ourselves
	if(node.Id() == BitTorrentManager().DhtTaskManager().Dht().LocalNodeId())
	{
		return false;
	}

	u32 distanceToNode = DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), node.Id());
	bool nodeIsClose = (distanceToNode <= CLOSE_NODE_DISTANCE);


	if(nodeIsClose)
	{
		if(NumberOfCloseNodes() < NUM_CLOSE_NODES_TO_KEEP)
		{
			InsertNode(node);
			return true;
		}
		else
		{
			//DebugPrint();

			// keep the closest nodes by throwing out further away ones as we go along		
			cDhtNode furthestCloseNode;
			u32 furthestCloseNodeDistance;
			FurthestCloseNodeDistance(&furthestCloseNode, &furthestCloseNodeDistance);

			if(furthestCloseNodeDistance != cDhtNode::INVALID_NODE_DISTANCE &&
			   distanceToNode < furthestCloseNodeDistance)
			{
				//Printf("DHT: Throwing away close node with distance %u for one with distance %u\n", furthestCloseNodeDistance, distanceToNode);
				bool ret = RemoveNode(furthestCloseNode);
				assert(ret);
				InsertNode(node);

				/*
				// ********************* VERY TEMP !!!!!!! ************************************************************
				static int xx=0;
				xx++;
				if(xx=32)
				{
					xx=0;
					Save();
				}
				*/
			}
		}
	}
	else
	{
		if(NumberOfFarNodes() < NUM_FAR_NODES_TO_KEEP)
		{
			InsertNode(node);
			return true;
		}
		else
		{
			// TODO : could randomly discard & replace far nodes here?
		}
	}
	return false;
}// END PresentNode



u32 cDhtRoutingTable::NumberOfCloseNodes() const
{
	u32 count=0;
	for(RoutingTableConstIterator iter = mRoutingTable.begin(); iter != mRoutingTable.end(); iter++)
	{
		if (DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), (*iter).Id()) <= CLOSE_NODE_DISTANCE)
		{
			count++;
		}
	}
	return count;
}// END NumberOfCloseNodes



u32 cDhtRoutingTable::NumberOfFarNodes() const
{
	return static_cast<u32>(mRoutingTable.size()) - NumberOfCloseNodes();
}// END NumberOfFarNodes



bool cDhtRoutingTable::HasEnoughCloseNodes() const
{
	return NumberOfCloseNodes() >= NUM_CLOSE_NODES_TO_KEEP;
}// END HasEnoughCloseNodes



void cDhtRoutingTable::FurthestCloseNodeDistance(cDhtNode* pNodeOut, u32* distanceOut) const
{
	DhtNodePtrVector closeNodes;
	CloseNodes(&closeNodes);

	if(closeNodes.size() == 0)
	{
		*distanceOut = cDhtNode::INVALID_NODE_DISTANCE;
		return;
	}

	const cDhtNode* furthestCloseNode = closeNodes[closeNodes.size()-1];
	u32 furthestCloseNodeDistance = DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), furthestCloseNode->Id());
	
	*pNodeOut = *furthestCloseNode;
	*distanceOut = furthestCloseNodeDistance;
}// END FurthestCloseNodeDistance



bool cDhtRoutingTable::InsertNode(const cDhtNode& node)
{	
	if(!node.Id().IsValid())
	{
		assert(0);
		return false;
	}

	bool hasNode = Node(node.Id())!=NULL;
	assert(!hasNode);
	if(hasNode)
	{
		return false;
	}

	assert(node.Id() != BitTorrentManager().DhtTaskManager().Dht().LocalNodeId());

	mRoutingTable.push_back(node);
	mRoutingTable.sort();

	//Printf("DHT: RTable Add [%s] - size [%u]\n", node.Address().Ip().AsString(), Size());

	//DebugPrint();

	return true;
}// END InsertNode



bool cDhtRoutingTable::RemoveNode(const cDhtNodeId& nodeId)
{
	RoutingTableIterator nodeIter = NodeIter(nodeId);
	if(nodeIter == mRoutingTable.end())
	{
		return false;
	}
	mRoutingTable.erase(nodeIter);
	
	return true;
}// END RemoveNode



bool cDhtRoutingTable::RemoveNode(const cDhtNode& node)
{
	return RemoveNode(node.Id());;
}// END RemoveNode



cDhtRoutingTable::RoutingTableIterator cDhtRoutingTable::NodeIter(const cDhtNodeId& nodeId)
{
	for(RoutingTableIterator iter = mRoutingTable.begin(); iter != mRoutingTable.end(); iter++)
	{
		if((*iter).Id().IsValid() &&
		   (*iter).Id() == nodeId)
		{
			return iter;
		}
	}
	return mRoutingTable.end();
}// END NodeIter



const cDhtNode* cDhtRoutingTable::Node(const cDhtNodeId& nodeId)
{
	RoutingTableIterator nodeIter = NodeIter(nodeId);
	if(nodeIter == mRoutingTable.end())
	{
		return NULL;
	}
	else
	{
		return &(*nodeIter);
	}
}// END Node



cDhtNode* cDhtRoutingTable::Node(const cSockAddr& sockAddr)
{
	for(RoutingTableIterator iter = mRoutingTable.begin(); iter != mRoutingTable.end(); iter++)
	{
		if((*iter).Address() == sockAddr)
		{
			return &(*iter);
		}
	}
	return NULL;
}// END Node



bool cDhtRoutingTable::Node(u32 nodeIndex, cDhtNode* nodeOut)
{
	ASSERT(nodeOut);
	if(mRoutingTable.size() < nodeIndex)
	{
		return false;
	}

	u32 i=0;
	for(RoutingTableIterator iter = mRoutingTable.begin(); iter != mRoutingTable.end(); iter++)
	{
		if(i == nodeIndex)
		{
			*nodeOut = (*iter);
			return true;
		}
		i++;
	}
	return false;
}// END Node



void cDhtRoutingTable::AllNodesWithInvalidId(DhtNodePtrVector* pOutVector) const
{
	pOutVector->clear();
	RoutingTableConstIterator constIter;
	for(constIter = mRoutingTable.begin(); constIter != mRoutingTable.end(); constIter++)
	{
		if((*constIter).Id().IsValid() == false)
		{
			pOutVector->push_back(&(*constIter));
		}
	}
}// END AllNodesWithInvalidId



// returns a vector containing the current routing table sorted by distance to our local node.
void cDhtRoutingTable::RoutingTableNodesSortedByDistanceToNode(const cDhtNodeId& node, DhtNodePtrVector* pOutVector) const
{
	DistanceFuncObj_ForNodePtrs cb(node);
	pOutVector->clear();

	RoutingTableConstIterator constIter;
	for(constIter = mRoutingTable.begin(); constIter != mRoutingTable.end(); constIter++)
	{
		pOutVector->push_back(&(*constIter));
	}
	sort(pOutVector->begin(), pOutVector->end(), cb);
}// END RoutingTableNodesSortedByDistanceToNode



// returns an ordered vector with the required nodes
void cDhtRoutingTable::CloseNodes(DhtNodePtrVector* pOutVector) const
{
	DhtNodePtrVector sortedByDistanceToHashList;
	RoutingTableNodesSortedByDistanceToNode(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), &sortedByDistanceToHashList);	

	pOutVector->clear();
	DhtNodePtrVectorIterator iter;
	for(iter = sortedByDistanceToHashList.begin(); iter != sortedByDistanceToHashList.end(); iter++)
	{
		const cDhtNode* node = *iter;
		
		u32 distanceToNode = DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), node->Id());
		bool nodeIsClose = (distanceToNode <= CLOSE_NODE_DISTANCE);
		if(nodeIsClose)
		{
			pOutVector->push_back(node);
		}
	}
}// END CloseNodes



void cDhtRoutingTable::FarNodes(DhtNodePtrVector* pOutVector) const
{
	DhtNodePtrVector sortedByDistanceToHashList;
	RoutingTableNodesSortedByDistanceToNode(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), &sortedByDistanceToHashList);	

	pOutVector->clear();
	DhtNodePtrVectorIterator iter;
	for(iter = sortedByDistanceToHashList.begin(); iter != sortedByDistanceToHashList.end(); iter++)
	{
		const cDhtNode* node = *iter;

		u32 distanceToNode = DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), node->Id());
		bool nodeIsClose = (distanceToNode > CLOSE_NODE_DISTANCE);
		if(nodeIsClose)
		{
			pOutVector->push_back(node);
		}
	}
}// END FarNodes



// When a node wants to find peers for a torrent, it uses the distance metric to compare the infohash of the torrent with the IDs of the nodes in its own routing table. 
// It then contacts the nodes it knows about with IDs closest to the infohash and asks them for the contact information of peers currently downloading the torrent.
void cDhtRoutingTable::ClosestNodeToInfoHash(const cDhtResourceId& resourceId, u32 maxNodesToReturn, DhtNodePtrVector* pOutVector) const
{	
	DhtNodePtrVector sortedByDistanceToHashList;
	RoutingTableNodesSortedByDistanceToNode(resourceId, &sortedByDistanceToHashList);	

	pOutVector->clear();
	for(u32 i=0; i < maxNodesToReturn; i++)
	{
		if(i < sortedByDistanceToHashList.size())
		{
			pOutVector->push_back(sortedByDistanceToHashList[i]);
		}
	}
}// END ClosestNodeToInfoHash



void cDhtRoutingTable::DebugPrint() const
{
	DhtNodePtrVector sortedByDistanceToHashList;
	RoutingTableNodesSortedByDistanceToNode(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), &sortedByDistanceToHashList);	

	Printf("***** Dht routing table *****\n\n");
	u32 i=0;
	for(DhtNodePtrVectorIterator iter = sortedByDistanceToHashList.begin(); iter != sortedByDistanceToHashList.end(); iter++)
	{
		u32 distanceToNode = DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), (*iter)->Id());
		Printf("Node %d -	%s:%u		-	Distance %u\n", i, (*iter)->Address().Ip().AsString(), (*iter)->Address().Port(), distanceToNode);
		i++;
	}
	Printf("*****************************\n\n");
}// END DebugPrint