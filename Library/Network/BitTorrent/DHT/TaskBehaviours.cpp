// Jon Bellamy 02/12/2009
// When responses to a tasks krpc message comes in, the task will have been setup to call one of these callbacks 
// which will shape how the task proceeds with its long term goal


#include "TaskBehaviours.h"

#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/BitTorrent/dht/Dht.h"
#include "Network/BitTorrent/dht/DhtTask.h"



// Upon inserting the first node into its routing table and when starting up thereafter, the node 
// should attempt to find the closest nodes in the DHT to itself. It does this by issuing find_node messages 
// to closer and closer nodes until it cannot find any closer. The routing table should be saved between 
// invocations of the client software.
void FindNodeBehaviour_SearchForCloserNodes(cDhtTask* pTask, DhtNodeVector* pResponseNodes, void* param)
{
	cDhtFindNodeTask* pFindNodeTask = static_cast<cDhtFindNodeTask*> (pTask);

	// all nodes in pResponseNodes are guaranteed to not already be in the tasks query queue
	
	for(u32 i=0; i < pResponseNodes->size(); i++)
	{
		cDhtNode& node = (*pResponseNodes)[i];

		// a node actually has us, ignore it
		if(node.Id() == BitTorrentManager().DhtTaskManager().Dht().LocalNodeId())
		{
			continue;
		}

		// get the furthest closest node, ie the next one we will drop when we get a closer node
		cDhtNode furthestCloseNode;
		u32 furthestCloseNodeDistance;
		BitTorrentManager().DhtTaskManager().Dht().RoutingTable().FurthestCloseNodeDistance(&furthestCloseNode, &furthestCloseNodeDistance);
		
		u32 distanceToResponseNode = DistanceBetweenDhtNodes(BitTorrentManager().DhtTaskManager().Dht().LocalNodeId(), node.Id());

		// add nodes that are closer than what we already have, they are the most likely to have more nodes that are closer to us
		if( !BitTorrentManager().DhtTaskManager().Dht().RoutingTable().HasEnoughCloseNodes() || 
			distanceToResponseNode <= furthestCloseNodeDistance)
		{
			pFindNodeTask->AddNodeToQuery(node);
		}
	}
}// END FindNodeBehaviour_SearchForCloserNodes


