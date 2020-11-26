// Jon Bellamy 02/12/2009
// When responses to a tasks krpc message comes in, the task will have been setup to call one of these callbacks 
// which will shape how the task proceeds with its long term goal


#ifndef DHT_TASK_BEHAVIOURSR__H_
#define DHT_TASK_BEHAVIOURSR__H_


#if USE_PCH
#include "stdafx.h"
#endif


#include "Network/BitTorrent/dht/RoutingTable.h"


class cDhtTask;



// used for exhaustive FindNode behaviour to find nodes 'close' to us
extern void FindNodeBehaviour_SearchForCloserNodes(cDhtTask* pTask, DhtNodeVector* pResponseNodes, void* param);




#endif // DHT_TASK_BEHAVIOURSR__H_