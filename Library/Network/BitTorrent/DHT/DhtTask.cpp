// Jon Bellamy 23/11/2009



#include "DhtTask.h"


#include <assert.h>
#include <algorithm>


#include "General/Endianness.h"
#include "Network/SockAddr.h"
#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/BitTorrent/dht/Dht.h"
#include "Network/BitTorrent/dht/DhtTaskManager.h"
#include "Network/BitTorrent/dht/SortObjects.h"


using namespace net;


cDhtTask::cDhtTask()
: mRunning(true)
, mOutstandingQueries(0)
, mCompleteCb(NULL)
, mCompleteCbParam(NULL)
{
}// END cDhtTask



cDhtTask::~cDhtTask()
{
	mRunning = (bool)0xDD;
	mCompleteCb = (DhtTaskCompleteCb) 0xDD;
	mCompleteCbParam = (void*) 0xDD;
	mOutstandingQueries = 0xDDDDDDDD;
}// END ~cDhtTask



// the Dht has given up on this query
void cDhtTask::OnTaskResponseTimeout(const cKrpcQuery& sent)
{
	mOutstandingQueries--;
}// END OnQueryResponseTimeout



void cDhtTask::OnTaskComplete()
{
	//Printf("DHT: cDhtTask::OnTaskComplete\n");
	if(mCompleteCb)
	{
		mCompleteCb(this, mCompleteCbParam);
	}

	// warning this may delete 'this', no code after here!
	BitTorrentManager().DhtTaskManager().OnTaskComplete(this);
}// END OnTaskComplete



cDhtQueryTask::cDhtQueryTask()
{
}// END cDhtTask



// the Dht has given up on this query
void cDhtQueryTask::OnTaskResponseTimeout(const cKrpcQuery& sent)
{
	cDhtTask::OnTaskResponseTimeout(sent);

	ProcessJobs();

	if(IsTaskComplete())
	{
		OnTaskComplete();
	}
}// END OnQueryResponseTimeout




//////////////////////////////////////////////////////////////////////////
// Ping


cDhtPingTask::cDhtPingTask(const cSockAddr& addrToPing, DhtTaskCompleteCb completeCb, void* completeCbParam)
: mAddrToPing(addrToPing)
, mPingSent(false)
{
	mCompleteCb = completeCb;
	mCompleteCbParam = completeCbParam;
}// END cDhtPingTask



void cDhtPingTask::OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response)
{
	if(mOutstandingQueries != 1)
	{
		assert(0);
		return;
	}

	mOutstandingQueries--;

	const cBencodedString* id = dynamic_cast<const cBencodedString*> (response.GetResponseValue("id"));
	if(id == NULL)
	{
		return;
	}

	if(id->Get().size() != cDhtNodeId::NODE_ID_SIZE)
	{
		assert(0);
		return;
	}

	cDhtNode node(sent.SentTo().Address(), id->Get());
	BitTorrentManager().DhtTaskManager().Dht().RoutingTable().PresentNode(node);


	OnTaskComplete();
}// END OnResponse



bool cDhtPingTask::IsTaskComplete() const 
{
	return (mPingSent == true && mOutstandingQueries == 0);
}// END IsTaskComplete



void cDhtPingTask::ProcessJobs() 
{
	BitTorrentManager().DhtTaskManager().Dht().PingNode(mAddrToPing, this);	
	mPingSent=true;
	assert(mOutstandingQueries == 0);
	mOutstandingQueries++;
}// END ProcessJobs



void cDhtPingTask::OnTaskResponseTimeout(const cKrpcQuery& sent)
{
	cDhtTask::OnTaskResponseTimeout(sent);
	assert(mOutstandingQueries == 0);
	OnTaskComplete();
}// END OnTaskResponseTimeout




//////////////////////////////////////////////////////////////////////////
// Announce


cDhtAnnounceTask::cDhtAnnounceTask()
: mLastSendTime(0)
{
}// END cDhtAnnounceTask



void cDhtAnnounceTask::Announce(const cDhtResourceId& resourceId, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	mCompleteCb = completeCb;
	mCompleteCbParam = completeCbParam;
	mResourceId = resourceId;
	mOutstandingQueries = 0;
	mLastSendTime = 0;

	// build the list of nodes we will announce to
	mAnnounceTargets.clear();
	DhtResourceInfoManager().AllPeersForResource(resourceId, &mAnnounceTargets, false);

	Printf("DHT: Announce to %u nodes\n", mAnnounceTargets.size());
}// END Announce



bool cDhtAnnounceTask::IsTaskComplete() const
{
	return (mAnnounceTargets.size() == 0 && mOutstandingQueries == 0);
}// END IsTaskComplete



void cDhtAnnounceTask::OnTaskResponseTimeout(const cKrpcQuery& sent)
{
	cDhtTask::OnTaskResponseTimeout(sent);

	if(IsTaskComplete())
	{
		OnTaskComplete();
	}
}// END OnTaskResponseTimeout



void cDhtAnnounceTask::ProcessJobs()
{
	if(BitTorrentManager().DhtTaskManager().Dht().Time() - mLastSendTime >= SEND_PERIOD)
	{
		// 22/12/2010 : Endian swap here?!?
		u16 DHT_PORT = BitTorrentManager().ListenPort();
		
		u32 numToSend = min(static_cast<u32>(mAnnounceTargets.size()), MAX_MSGS_SEND_ONE_SHOT);
		if(numToSend > 0)
		{
			//Printf("DHT: Announcing to port %u\n", DHT_PORT);

			for(u32 i=0; i < numToSend; i++)
			{
				cPeerWithResource& announceTarget = mAnnounceTargets.back();

				assert(announceTarget.mExternalToken.size() > 0);

				mOutstandingQueries++;

				BitTorrentManager().DhtTaskManager().Dht().AnnouncePeer(announceTarget.mNode, mResourceId, DHT_PORT, announceTarget.mExternalToken, this);				

				mAnnounceTargets.pop_back();
			}

			mLastSendTime = BitTorrentManager().DhtTaskManager().Dht().Time();
		}
	}
}// END ProcessJobs



void cDhtAnnounceTask::Process()
{
	cDhtTask::Process();
	if(IsRunning() &&
	   !IsTaskComplete() &&
	   BitTorrentManager().DhtTaskManager().Dht().Time() - mLastSendTime >= SEND_PERIOD)
	{
		ProcessJobs();
	}
}// END Process



void cDhtAnnounceTask::OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response) 
{ 
	//Printf("DHT: cDhtAnnounceTask::OnResponse\n");
	mOutstandingQueries--;

	// response to an announce seems to be just the reply nodes id
	const cBencodedString* pbStr = dynamic_cast<const cBencodedString*> (response.GetResponseValue("id"));
	if(!pbStr ||
	   sent.SentTo().Id().AsString() != pbStr->Get())
	{
		Printf("DHT: Strange announce id\n");
	}

	if(IsTaskComplete())
	{
		OnTaskComplete();
	}
}// END OnResponse



//////////////////////////////////////////////////////////////////////////
// Get Peers



cDhtGetPeersTask::cDhtGetPeersTask()
: mNodeCountToStartWith(0)
{
	// reserve a decent chunk, these queries grow pretty fast
	mSearchHistory.reserve(512);
}// END cDhtGetPeersTask



void cDhtGetPeersTask::DebugPrintQueryQueue() const
{
	for(u32 i=0; i < mNodesToQuery.size(); i++)
	{
		const cDhtNode& node = mNodesToQuery[i];

		u32 dist = DistanceBetweenDhtNodes(node.Id(), mResourceId);
		Printf("DHT: Node %d distance[%u]\n", i, dist);
	}
}// END DebugPrintQueryQueue



void cDhtGetPeersTask::GetPeersForResource(const cDhtResourceId& resId, u32 numNodesToStartWith, DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	mCompleteCb = completeCb;
	mCompleteCbParam = completeCbParam;

	mNodeCountToStartWith = numNodesToStartWith;

	mResourceId = resId;

	//cDhtNodeId nid;
	//mResourceId = nid;

	mNodesToQuery.clear();
	mSearchHistory.clear();
	mPeersWithResource.clear();
	

	// get the nearest nodes
	DhtNodePtrVector closestNodes;
	BitTorrentManager().DhtTaskManager().Dht().RoutingTable().ClosestNodeToInfoHash(mResourceId, numNodesToStartWith, &closestNodes);

	

	for(u32 i=0; i < closestNodes.size(); i++)
	{
		const cDhtNode* pNode = closestNodes[i];
		mNodesToQuery.push_back(*pNode);
	}


	//DebugPrintQueryQueue();


	if(closestNodes.size() == 0)
	{
		OnTaskComplete();
	}
	else
	{
		ProcessJobs();
	}
}// END GetPeersForResource



void cDhtGetPeersTask::Restart()
{	
	GetPeersForResource(mResourceId, mNodeCountToStartWith, mCompleteCb, mCompleteCbParam);
}// END Restart



u32 cDhtGetPeersTask::InjectExtraSearchQueries(u32 numToAdd)
{
	// get all the routing table nodes ordered by nearest to resource
	DhtNodePtrVector closestNodes;
	BitTorrentManager().DhtTaskManager().Dht().RoutingTable().ClosestNodeToInfoHash(mResourceId, BitTorrentManager().DhtTaskManager().Dht().RoutingTable().Size(), &closestNodes);

	// add them in order as long as they are not already in the search history
	u32 added=0;
	for(u32 i=0; i < closestNodes.size(); i++)
	{
		const cDhtNode* node = closestNodes[i];

		if(find(mSearchHistory.begin(), mSearchHistory.end(), *node) == mSearchHistory.end())
		{
			mNodesToQuery.push_back(*node);
			added++;
		}

		if(added >= numToAdd)
		{
			break;
		}
	}

	// sort the 'nodes to query' by their 'distance' to the resource we are after
	DistanceFuncObj_ForNodes cb(mResourceId);
	sort(mNodesToQuery.begin(), mNodesToQuery.end(), cb);

	//DebugPrintQueryQueue();

	return added;
}// END InjectExtraSearchQueries



void cDhtGetPeersTask::ProcessJobs()
{
	// Cannot let the search history get too big or memory will become an issue, this is already 416Kb!
	if(mSearchHistory.size() >= MAX_SEARCH_HISTORY_SIZE)
	{
		mSearchHistory.clear();
	}

	while(IsRunning() &&
		  mOutstandingQueries < MAX_OUTSTANDING_QUERIES &&
		  mNodesToQuery.size() > 0)
	{
		// take the next node to search for out of the search queue and put it into the search history
		cDhtNode& node = mNodesToQuery.front();
		
		// TODO : This send can fail but it was then creating an infinite loop if it did and we checked for it.
		// HACK : removed send fail check to prevent infinite loop
		// TODO : SORT ME !!!!
		BitTorrentManager().DhtTaskManager().Dht().GetPeers(node, mResourceId, this);
		mOutstandingQueries++;
	
		mSearchHistory.push_back(node);
		mNodesToQuery.pop_front();
	}
}// END ProcessQueryQueue



bool cDhtGetPeersTask::IsTaskComplete() const
{
	return (mNodesToQuery.size() == 0  && mOutstandingQueries == 0);
}// END SearchExhausted



void cDhtGetPeersTask::SortQueryQueue()
{
	// sort the 'nodes to query' by their 'distance' to the resource we are after
	DistanceFuncObj_ForNodes cb(mResourceId);
	sort(mNodesToQuery.begin(), mNodesToQuery.end(), cb);
}// END SortQueryQueue



void cDhtGetPeersTask::OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response)
{
	mOutstandingQueries--;


	// TODO : ALL of these ben_type casts are potential crashes, we MUST use a guard cast and a type guard check on all of them!!!!!!!!!!!!!
	// #define BEN_CAST(ptr, btype) static_cast<btye> (ptr); assert(ptr);
	// or dynamic_cast????
	const cBencodedString* token = dynamic_cast<const cBencodedString*> (response.GetResponseValue("token"));
	if(!token)
	{
		return;
	}

	const cBencodedList* peerList = dynamic_cast<const cBencodedList*> (response.GetResponseValue("values"));
	if(peerList)
	{		
		// found someone with the resource, these peers all have the resource we have requested

		Printf("DHT: Found peer with resource\n");

		// update our resource info database, only going to keep a record of people who know about the resource
		DhtResourceInfoManager().OnGetPeersResponse(sent.SentTo(), mResourceId, token->Get());

		for(u32 i=0; i < peerList->NumElements(); i++)
		{
			const cBencodedString* pPeerString = dynamic_cast<const cBencodedString*> (peerList->GetElement(i));
			if( pPeerString == NULL ||
				pPeerString->Type() != cBencodedType::BEN_STRING)
			{
				break;
			}

			const u8* compactData = reinterpret_cast<const u8*> (pPeerString->Get().c_str());

			// NB : These are peers not nodes. The contact info returned here is where the peer
			// is listening for incoming TCP BT connections
			cIpAddr ip(compactData[0], compactData[1], compactData[2], compactData[3]);
			u16 port;
			memcpy(&port, &compactData[4], sizeof(port));
			endian_swap(port);

			// does this peer have the resource
			if(ip == sent.SentTo().Address().Ip())
			{
			}

			cSockAddr addr(ip, port);
			mPeersWithResource.push_back(addr);			
			
			// TODO : found callback ...
		}
	}
	else
	{
		// node doesn't know anyone who has the resource, so its sent us a list of the 'closest' nodes it has

		// NB : i've seen messages with 'nodes2'
		if(response.GetResponseValue("nodes"))
		{
			if( response.GetResponseValue("nodes")->Type() != cBencodedType::BEN_STRING )
			{
				// This has been seen to contain a list type here & then crash below when casted to a string
				// TODO : handle the list type.
				Printf("Dht: Get peers response which contains a 'nodes' element which is not a string");
			}
			else
			{
				assert(response.GetResponseValue("nodes")->Type() == cBencodedType::BEN_STRING);		
				const cBencodedString* pPeersStr = dynamic_cast<const cBencodedString*> (response.GetResponseValue("nodes"));
				if(pPeersStr)
				{
					// these are the 'closest' nodes to the resource hash
					const std::string& peersStr = pPeersStr->Get();
					u32 numPeers = static_cast<u32>(peersStr.size()) / cDht::COMPACT_NODE_REPRESENTATION_SIZE;
					
					// Catch silly values, which will probably crash below, nobody is returning more than 50
					assert(numPeers <= 50);

					const u8* compactData = reinterpret_cast<const u8*> (peersStr.c_str());

					for(u32 i=0; i < numPeers; i++)
					{
						cDhtNode node;
						node.FromCompactRepresentation(&compactData[i*cDht::COMPACT_NODE_REPRESENTATION_SIZE]);

						BitTorrentManager().DhtTaskManager().Dht().RoutingTable().PresentNode(node);

						// check the search history & upcoming search list
						if(find(mSearchHistory.begin(), mSearchHistory.end(), node) != mSearchHistory.end() ||
						   find(mNodesToQuery.begin(), mNodesToQuery.end(), node) != mNodesToQuery.end())
						{
							continue;
						}


						// Accept all nodes at first so we have a healthy queue to process
						if(NumberOfPeersFound() == 0 && mNodesToQuery.size() < HEALTHY_ACTIVE_QUERY_QUEUE_SIZE)
						{
							mNodesToQuery.push_back(node);	

							SortQueryQueue();
						}			
						else
						{	
							// When we get here, we have found some peers with the resource and are now only willing to accept 
							// nodes that are closer to the resource than the ones we have. This allows queries to complete, if 
							// the user wants to keep searching after we complete then they can always call ExpandTaskSearch.

							if(mNodesToQuery.size() > 0)
							{
								// we have a healthy queue, just accept closer nodes
								const cDhtNode& furthestNode = mNodesToQuery.back();
								u32 distanceToFurthestNode = DistanceBetweenDhtNodes(furthestNode.Id(), mResourceId);
								u32 distanceToNewNode = DistanceBetweenDhtNodes(node.Id(), mResourceId);
								if(distanceToNewNode < distanceToFurthestNode)
								{
									mNodesToQuery.pop_back();
									mNodesToQuery.push_back(node);

									SortQueryQueue();
								}
							}
						}
					}

					// WARNING : this is going to announce to the every node that responded to us in the search
					// update our resource info database, only going to keep a record of people who know about the resource
					DhtResourceInfoManager().OnGetPeersResponse(sent.SentTo(), mResourceId, token->Get());

					//DebugPrintQueryQueue();
					
				}
			}
		}
	}

	ProcessJobs();

	if(IsTaskComplete())
	{
		OnTaskComplete();
	}
}// END OnResponse




//////////////////////////////////////////////////////////////////////////
// Find Node


cDhtFindNodeTask::cDhtFindNodeTask()
: mBehaviourCb(NULL)
, mBehaviourCbParam(NULL)
, mNextIndexToQuery(0)
{
	// reserve a decent chunk, these queries grow pretty fast
	mNodesToQuery.reserve(256);
}// END cDhtFindNodeTask



void cDhtFindNodeTask::FindNode(const cDhtNodeId& nodeId, DhtTaskBehaviourCb BehaviourCb, void* behaviourCbParam, DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	mSearchNodeId = nodeId;

	mCompleteCb = completeCb;
	mCompleteCbParam = completeCbParam;

	mBehaviourCb = BehaviourCb;
	mBehaviourCbParam = behaviourCbParam;


	mNodesToQuery.clear();
	mNextIndexToQuery =0;

	// TODO : maybe query all nodes (Dht().RoutingTable().Size()) here?

	// get the nearest nodes
	DhtNodePtrVector closestNodes;
	BitTorrentManager().DhtTaskManager().Dht().RoutingTable().ClosestNodeToInfoHash(nodeId, /*BitTorrentManager().DhtTaskManager().Dht().RoutingTable().Size()*/ cDht::NODE_COUNT_FOR_QUERY_AND_RETURN, &closestNodes);

	cDhtNode tempNode;
	if(closestNodes.size() == 0)
	{
		ASSERT_MSG(BitTorrentManager().DhtTaskManager().Dht().RoutingTable().Size() != 0, "Empty routing table? Stack is about to blow.");
		BitTorrentManager().DhtTaskManager().Dht().RoutingTable().Node(0, &tempNode);
		closestNodes.push_back(&tempNode);

		// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// TODO : This can AND WILL cause a stack overflow, see cDht::BuildRoutingTableTaskCompleteCb
		//OnTaskComplete();
	}
	
	for(u32 i=0; i < closestNodes.size(); i++)
	{
		// TODO : bootstrap nodes can have no id while we wait for ping response, leave this out for now
		//if(closestNodes[i]->Id().IsValid())
		{
			mNodesToQuery.push_back(*(closestNodes[i]));
		}
	}

	ProcessJobs();
}// END FindNode



void cDhtFindNodeTask::Restart()
{
	FindNode(mSearchNodeId, mBehaviourCb, mBehaviourCbParam, mCompleteCb, mCompleteCbParam);
}// END Restart



u32 cDhtFindNodeTask::InjectExtraSearchQueries(u32 numToAdd)
{
	// TODO
	assert(0);
	return 0;
}// END InjectExtraSearchQueries



void cDhtFindNodeTask::ProcessJobs()
{
	while(IsRunning() &&
		mOutstandingQueries < MAX_OUTSTANDING_QUERIES &&
		mNodesToQuery.size() > 0 &&
		mNextIndexToQuery < mNodesToQuery.size())
	{
		// TODO : This send can fail but it was then creating an infinite loop if it did and we checked for it.
		// HACK : removed send fail check to prevent infinite loop
		// TODO : SORT ME !!!!
		BitTorrentManager().DhtTaskManager().Dht().FindNode(mNodesToQuery[mNextIndexToQuery], mSearchNodeId, this);
		mOutstandingQueries++;	
		mNextIndexToQuery++;
	}
}// END ProcessQueryQueue



// once the local routing table is full, start throwing away far away nodes we have in our up coming query list
// this keeps our up coming query list size reasonable and means we only query closer & closer nodes
void cDhtFindNodeTask::TrimUpcomingQueryList()
{
	if(BitTorrentManager().DhtTaskManager().Dht().RoutingTable().HasEnoughCloseNodes() == false)
	{
		return;
	}
	
	Printf("DHT: Upcoming queries before trim %d\n", mNodesToQuery.size() - mNextIndexToQuery);

	// get the furthest closest node, ie the next one we will drop when we get a closer node
	cDhtNode furthestCloseNode;
	u32 furthestCloseNodeDistance;
	BitTorrentManager().DhtTaskManager().Dht().RoutingTable().FurthestCloseNodeDistance(&furthestCloseNode, &furthestCloseNodeDistance);

	Functor_NodeCloserThan cb(furthestCloseNodeDistance, SearchTarget());
	remove_if(mNodesToQuery.begin()+mNextIndexToQuery, mNodesToQuery.end(), cb);

	Printf("DHT: Upcoming queries after trim %d\n", mNodesToQuery.size() - mNextIndexToQuery);
}// END TrimUpcomingQueryList



// the task manager has suspended us (good time to do house keeping)
void cDhtFindNodeTask::OnSuspend()
{
	TrimUpcomingQueryList();
}// END OnSuspend



bool cDhtFindNodeTask::IsTaskComplete() const
{
	return (mNodesToQuery.size() <= mNextIndexToQuery && mOutstandingQueries == 0);
}// END SearchExhausted



void cDhtFindNodeTask::OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response)
{
	mOutstandingQueries--;

	// I've seen 'nodes2', should probably look for it
	if(response.GetResponseValue("nodes") == NULL)
	{
		return;
	}

	assert(response.GetResponseValue("nodes")->Type() == cBencodedType::BEN_STRING);		
	const cBencodedString* pPeersStr = dynamic_cast<const cBencodedString*> (response.GetResponseValue("nodes"));
	if(pPeersStr)
	{
		DhtNodeVector returnedNodes;
		
		// get the returned nodes, build the list and send it to the behaviour callback
		const std::string& peersStr = pPeersStr->Get();
		u32 numPeers = static_cast<u32>(peersStr.size()) / cDht::COMPACT_NODE_REPRESENTATION_SIZE;

		const u8* compactData = reinterpret_cast<const u8*> (peersStr.c_str());

		for(u32 i=0; i < numPeers; i++)
		{
			cDhtNode node;
			node.FromCompactRepresentation(&compactData[i*cDht::COMPACT_NODE_REPRESENTATION_SIZE]);


			// check if they have us
			//if(node.Id() == BitTorrentManager().DhtTaskManager().Dht().LocalNodeId())
			//{
			//	assert(numPeers == 1);
			//	return;
			//}
			
			BitTorrentManager().DhtTaskManager().Dht().RoutingTable().PresentNode(node);

			// don't add dupes
			if(find(mNodesToQuery.begin(), mNodesToQuery.end(), node) == mNodesToQuery.end())
			{
				returnedNodes.push_back(node);
			}
		}

		if(mBehaviourCb)
		{
			// i'm calling out here so that i can provide different behaviours for this type of task while keeping the core parsing here
			mBehaviourCb(this, &returnedNodes, mBehaviourCbParam);
		}	

		//BitTorrentManager().DhtTaskManager().Dht().RoutingTable().DebugPrint();
	}

	ProcessJobs();

	if(IsTaskComplete())
	{
		OnTaskComplete();
	}
}// END OnResponse






