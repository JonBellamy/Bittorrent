// Jon Bellamy 18/11/2009
// http://bittorrent.org/beps/bep_0005.html

// boot strap nodes:

// router.bittorrent.com:6881/  
// router.utorrent.com:6881/
// router.bitcomet.net:554/

// try ...
// torrent.ubuntu.com:6969/ 


#include "Dht.h"

#include <assert.h>

#include "Network/BitTorrent/dht/DhtTaskManager.h"
#include "Network/BitTorrent/dht/TaskBehaviours.h"
#include "Network/BitTorrent/BitTorrentManager.h"
#include "Network/NetworkAdaptorList.h"
#include "Network/SockAddr.h"
#include "Network/Dns.h"
#include "General/Endianness.h"


#define BOOTSTRAP_NODE_A "router.bittorrent.com"
#define BOOTSTRAP_NODE_B "router.utorrent.com"
#define BOOTSTRAP_NODE_C "router.bitflu.org"



using namespace net;



cDht::cDht()
: mSocket(INVALID_SOCKET, SOCK_RECV_BUFFER_SIZE, SOCK_SEND_BUFFER_SIZE)
, mTimer(cTimer::STOPWATCH)
, mUdpDatagramsSent(0)
, mUdpDatagramsNoReply(0)
, mUdpUnmatchedResponsesRcvd(0)
, mGetPeersMsgsSent(0)
, mGetPeersMsgsResponseRcvd(0)
{
}// END cDht



void cDht::Init()
{
#if LOG_DHT_MESSAGES
	mPacketLog.Init(BitTorrentManager().DhtLogFolder());
	mPacketLog.Open("DhtPackertLog.html");
#endif

	WinSock().Open();
	cIpAddr localIp;
	localIp = NetworkAdaptorList().GetAdaptorIp(0);
	mLocalNode = cDhtNode(cSockAddr(localIp, DEFAULT_DHT_PORT));
	
	mSocket.Open(cSockAddr(localIp, DEFAULT_DHT_PORT), false);

	// TODO : when we load we have a new local node id so we are probably loading lots of far nodes which we then cannot get rid of. BIG TODO, get rid of them somehow
	mRoutingTable.Load();

	// TODO : this is reading the bootstraps as they have no nodeid at this point !!
	LoadBootStrapNodes(NULL);

	mRoutingTable.DebugPrint();

	mTimer.Reset();
	mTimer.Start();
}// END Init



void cDht::DeInit()
{
	mRoutingTable.Save();

	mSocket.Close();

	mTimer.Stop();
	memset(mRecvBuffer, 0, sizeof(mRecvBuffer));

	mUdpDatagramsSent = 0;
	mUdpDatagramsNoReply = 0;
	mUdpUnmatchedResponsesRcvd = 0;
	mGetPeersMsgsSent = 0;
	mGetPeersMsgsResponseRcvd = 0;

	// used to find nodes 'close' to us
	if(mpBuildRoutingTableTask)
	{
		BitTorrentManager().DhtTaskManager().DeleteTask(mpBuildRoutingTableTask); 
		mpBuildRoutingTableTask=NULL;
	}
	
	if(mOutstandingQueries.empty() == false)
	{
		for(OutstandingQueryList_Iterator iter = mOutstandingQueries.begin(); iter != mOutstandingQueries.end(); iter++)
		{
			const cKrpcQuery* krpcMsg = *iter;
			delete krpcMsg;
		}
		mOutstandingQueries.clear();
	}


	mRoutingTable.Clear();

#if LOG_DHT_MESSAGES
	mPacketLog.Close();
#endif
}// END DeInit



// TODO : read these from a file
void cDht::LoadBootStrapNodes(const char* bootStrapFilename)
{	
	cIpAddr bootstrapIp;
	cSockAddr bootStrapAddr;
	
	/*
	unsigned char dat[16];
	dat[0] = 0x4d;
	dat[1] = 0xd2;
	dat[2] = 0x8f;
	dat[3] = 0x8a;
	dat[4] = 0x1a;
	dat[5] = 0xe1;
	cSockAddr addr = SockAddrFromCompactRepresentation(dat);
	mRoutingTable.InsertNode(cDhtNode(addr, INVALID_NODE_ID));
	*/

	bool ret;
	u32 numBootstrap=0;

	ret = cDns::IpAddressFromDomainName(BOOTSTRAP_NODE_A, 0, &bootstrapIp);
	if(ret)
	{
		numBootstrap++;
		bootStrapAddr = cSockAddr(bootstrapIp, DEFAULT_DHT_PORT);
		BitTorrentManager().DhtTaskManager().PingNode(cDhtTaskManager::HIGH_PRIORITY, bootStrapAddr, NULL, NULL);
		//Printf("DHT: Bootstrap node : %s port %u\n", bootstrapIp.AsString(), DEFAULT_DHT_PORT);
	}


	ret = cDns::IpAddressFromDomainName(BOOTSTRAP_NODE_B, 0, &bootstrapIp);
	if(ret)
	{
		numBootstrap++;
		bootStrapAddr = cSockAddr(bootstrapIp, DEFAULT_DHT_PORT);
		BitTorrentManager().DhtTaskManager().PingNode(cDhtTaskManager::HIGH_PRIORITY, bootStrapAddr, NULL, NULL);
	}

	ret = cDns::IpAddressFromDomainName(BOOTSTRAP_NODE_C, 0, &bootstrapIp);
	if(ret)
	{
		numBootstrap++;
		bootStrapAddr = cSockAddr(bootstrapIp, DEFAULT_DHT_PORT);
		BitTorrentManager().DhtTaskManager().PingNode(cDhtTaskManager::HIGH_PRIORITY, bootStrapAddr, NULL, NULL);
	}

//	ret = cDns::IpAddressFromDomainName("router.bitcomet.net", 0, &bootstrapIp);
//	if(ret)
//	{
//		bootStrapAddr = cSockAddr(bootstrapIp, 554);
//		BitTorrentManager().DhtTaskManager().PingNode(cDhtTaskManager::HIGH_PRIORITY, bootStrapAddr, NULL, NULL);
//	}

	ret = cDns::IpAddressFromDomainName("torrent.ubuntu.com", 0, &bootstrapIp);
	if(ret)
	{
		numBootstrap++;
		bootStrapAddr = cSockAddr(bootstrapIp, 6969);
		BitTorrentManager().DhtTaskManager().PingNode(cDhtTaskManager::HIGH_PRIORITY, bootStrapAddr, NULL, NULL);
	}

	Printf("Dht: %d Bootstrap nodes loaded.\n", numBootstrap);
}// END LoadBootStrapNodes



void cDht::Process()
{
	mTimer.Process();

	mRoutingTable.Process();

	cSockAddr from;
	
	bool datagramRecvd = true;
	u32 datagramsProcessed = 0;
	
	// We can't totally drain this every time or we get long blocking periods in here.
	while(datagramRecvd == true &&
		  datagramsProcessed < 8)
	{
		s32 bytesRcvd = mSocket.Recv(mRecvBuffer, sizeof(mRecvBuffer), &from);
		assert(bytesRcvd <= RECV_BUFFER_SIZE);
	
		datagramRecvd = (bytesRcvd > 0);
		datagramsProcessed++;

		if(bytesRcvd < 0)
		{
			Printf("DHT: recv error %s:%u\n", from.Ip().AsString(), from.Port());
			//assert(0);
		}

		if(bytesRcvd > 0)
		{
			OnReceiveKrpcDatagram(from, reinterpret_cast<const char *> (mRecvBuffer), bytesRcvd);
		}
	}


	bool itemDeleted=false;
	do
	{
		itemDeleted=false;

		// TODO : We should only delete a node if it fails to respond to two concurrent requests!!!
		//  Timeout query responses
		for(OutstandingQueryList_Iterator iter = mOutstandingQueries.begin(); iter != mOutstandingQueries.end(); iter++)
		{
			const cKrpcQuery* krpcMsg = *iter;
			if(mTimer.ElapsedMs() - krpcMsg->SendTime() >= SENT_QUERY_TIMEOUT)
			{
				if(krpcMsg->GetTask())
				{
					krpcMsg->GetTask()->OnTaskResponseTimeout(*krpcMsg);
				}
				
				// remove them from the routing table
				mRoutingTable.RemoveNode(krpcMsg->SentTo());
				
				delete krpcMsg;
				mOutstandingQueries.erase(iter);

				mUdpDatagramsNoReply++;

				itemDeleted=true;
				break;
			}
		}
	}while(itemDeleted);
	

	// as soon as we have a routing table with some nodes, kick this task off
	if(mpBuildRoutingTableTask == NULL)
	{
		if(mRoutingTable.Size() != 0)
		{
			// submit a task to the dht, start finding nodes closest to us, this task can just slowly run forever, except when the 
			// routing table is not full in which case it runs with a higher priority  
			mpBuildRoutingTableTask = BitTorrentManager().DhtTaskManager().FindNode(cDhtTaskManager::HIGH_PRIORITY, LocalNodeId(), FindNodeBehaviour_SearchForCloserNodes, NULL, BuildRoutingTableTaskCompleteCb, NULL);
		}
	}
	else
	{
		// set the priority 
		cDhtTaskManager::TaskPriority priority = (mRoutingTable.Size() < (cDhtRoutingTable::NUM_CLOSE_NODES_TO_KEEP/2)) ? cDhtTaskManager::HIGH_PRIORITY : cDhtTaskManager::LOW_PRIORITY;
		if(BitTorrentManager().DhtTaskManager().GetTaskPriority(mpBuildRoutingTableTask) != priority)
		{
			Printf("DHT: Changing priority of mpBuildRoutingTableTask to %s\n", priority == cDhtTaskManager::HIGH_PRIORITY ? "HIGH_PRIORITY" : "LOW_PRIORITY");
			BitTorrentManager().DhtTaskManager().SetTaskPriority(mpBuildRoutingTableTask, priority);
		}
	}
}// END Process



void cDht::OnQueryResponseReceived(const cKrpcQuery& query)
{
	OutstandingQueryList_Iterator iter;
	for(iter = mOutstandingQueries.begin(); iter != mOutstandingQueries.end(); iter++)
	{
		if((*iter)->TransactionId() == query.TransactionId())
		{
			const cKrpcQuery* pKrpcQuery = *iter;
			// delete & remove from list
			delete pKrpcQuery;
			pKrpcQuery = NULL;
			mOutstandingQueries.erase(iter);
			return;
		}
	}
	assert(0);
}// END OnQueryResponseReceived



const cKrpcQuery* cDht::GetOutstandingMessage(u16 transactionId)
{
	OutstandingQueryList_Iterator iter;
	for(iter = mOutstandingQueries.begin(); iter != mOutstandingQueries.end(); iter++)
	{
		if((*iter)->TransactionId() == transactionId)
		{
			return *iter;
		}
	}
	return NULL;
}// END GetOutstandingMessage



void cDht::BuildRoutingTableTaskCompleteCb(const cDhtTask* pTask, void* param)
{
	assert(BitTorrentManager().DhtTaskManager().Dht().mpBuildRoutingTableTask && BitTorrentManager().DhtTaskManager().Dht().mpBuildRoutingTableTask == pTask && BitTorrentManager().DhtTaskManager().Dht().mpBuildRoutingTableTask->IsTaskComplete());
	
	// restart the find nodes task if it ends

	// Keep the bootstrap nodes in the routing table
	if(BitTorrentManager().DhtTaskManager().Dht().mRoutingTable.Size() == 0)
	{
		BitTorrentManager().DhtTaskManager().Dht().LoadBootStrapNodes(NULL);
	}

	
	// TODO : the task may well need to drain first!
	// TODO : This can and will blow the stack sometimes
	BitTorrentManager().DhtTaskManager().RestartTask(BitTorrentManager().DhtTaskManager().Dht().mpBuildRoutingTableTask);
}// END BuildRoutingTableTaskCompleteCb



bool cDht::PingNode(const cDhtNode& node, cDhtTask* pTask)
{
	//Printf("DHT: PingNode : %s:%u\n", node.Address().Ip().AsString(), node.Address().Port());

	cKrpcPingQuery* pingQuery = new cKrpcPingQuery(reinterpret_cast<const u8*> (mLocalNode.Id().AsString().c_str()));
	pingQuery->SetTask(pTask);
	
	u32 bytesSent = SendQuery(node, pingQuery);
	if(bytesSent == static_cast<u32>(pingQuery->Message().size()))
	{
		mOutstandingQueries.push_back(pingQuery);
		return true;
	}
	else
	{
		//Printf("DHT: PingNode send failed\n");
		delete pingQuery;
		return false;
	}
}// END PingNode



bool cDht::PingNode(const cSockAddr& addr, cDhtTask* pTask)
{
	cDhtNode node(addr);
	return PingNode(node, pTask);
}// END PingNode



// Find node is used to find the contact information for a node given its ID. "q" == "find_node" A find_node query has 
// two arguments, "id" containing the node ID of the querying node, and "target" containing the ID of the node sought 
// by the queryer. When a node receives a find_node query, it should respond with a key "nodes" and value of a string 
// containing the compact node info for the target node or the K (8) closest good nodes in its own routing table.
bool cDht::FindNode(const cDhtNode& node, const cDhtNodeId& target, cDhtTask* pTask)
{
	//Printf("DHT: cDht::FindNode - To %s:%d\n", node.Address().Ip().AsString(), node.Address().Port());

	cKrpcFindNodeQuery* pFindNodeQuery = new cKrpcFindNodeQuery(reinterpret_cast<const u8*>(mLocalNode.Id().AsString().c_str()), reinterpret_cast<const u8*>(target.AsString().c_str()));
	pFindNodeQuery->SetTask(pTask);
	
	u32 bytesSent = SendQuery(node, pFindNodeQuery);
	if(bytesSent == static_cast<u32>(pFindNodeQuery->Message().size()))
	{
		mOutstandingQueries.push_back(pFindNodeQuery);
		return true;
	}
	else
	{
		//Printf("DHT: FindNode send failed\n");
		delete pFindNodeQuery;
		return false;
	}
}// END FindNode



// Get peers associated with a torrent infohash. "q" = "get_peers" A get_peers query has two arguments, "id" containing the 
// node ID of the querying node, and "info_hash" containing the infohash of the torrent. If the queried node has peers for 
// the infohash, they are returned in a key "values" as a list of strings. Each string containing "compact" format peer 
// information for a single peer. If the queried node has no peers for the infohash, a key "nodes" is returned containing 
// the K nodes in the queried nodes routing table closest to the infohash supplied in the query. 
bool cDht::GetPeers(const cDhtNode& node, const cDhtResourceId& infoHash, cDhtTask* pTask)
{	
	assert(pTask);

	u32 distance = DistanceBetweenDhtNodes(node.Id(), infoHash);
	//Printf("DHT: Sending GetPeers To %s:%d Distance from resource (%d).", node.Address().Ip().AsString(), node.Address().Port(), distance);	
	//Printf(" Resource id - ");
	//infoHash.DebugPrint();
	//Printf("\n");

	cKrpcGetPeersQuery* pGpQuery = new cKrpcGetPeersQuery(reinterpret_cast<const u8*> (mLocalNode.Id().AsString().c_str()), reinterpret_cast<const u8*> (infoHash.AsString().c_str()));
	pGpQuery->SetTask(pTask);
	
	s32 bytesSent = SendQuery(node, pGpQuery);

	if(bytesSent == static_cast<u32>(pGpQuery->Message().size()))
	{
		mOutstandingQueries.push_back(pGpQuery);
		mGetPeersMsgsSent++;
		return true;
	}
	else
	{
		//Printf("DHT: GetPeers send failed\n");
		delete pGpQuery;
		return false;
	}
}// END GetPeers



// Announce that the peer, controlling the querying node, is downloading a torrent on a port. announce_peer has four arguments: 
// "id" containing the node ID of the querying node, "info_hash" containing the infohash of the torrent, "port" containing the port 
// as an integer, and the "token" received in response to a previous get_peers query. 
bool cDht::AnnouncePeer(const cDhtNode& to, const cDhtResourceId& infoHash, u16 port, const std::string& token, cDhtTask* pTask)
{
	assert(pTask);

	cKrpcAnnouncePeerQuery* pAnnounceQuery = new cKrpcAnnouncePeerQuery(reinterpret_cast<const u8*> (mLocalNode.Id().AsString().c_str()), reinterpret_cast<const u8*> (infoHash.AsString().c_str()), port, token);
	pAnnounceQuery->SetTask(pTask);

	//pAnnounceQuery->Save("my announce-sent.ben");

	//Printf("DHT: Sending AnnouncePeer. Transaction %u Port %u\n", pAnnounceQuery->TransactionId(), port);

	u32 bytesSent = SendQuery(to, pAnnounceQuery);

	if(bytesSent == static_cast<u32>(pAnnounceQuery->Message().size()))
	{
		mOutstandingQueries.push_back(pAnnounceQuery);
		return true;
	}
	else
	{
		assert(0);
		//Printf("DHT: AnnouncePeer send failed\n");
		delete pAnnounceQuery;
		return false;
	}
}// END AnnouncePeer


/*
// pauses all tasks and waits for all outstanding queries to come in or timeout
void cDht::DrainAllOutstandingQueries()
{
	BitTorrentManager().DhtTaskManager().SuspendAllTasks();
	BitTorrentManager().DhtTaskManager().AllowScheduler(false);
	//mDrainingDht = true;
	assert(0);
}// END DrainAllOutstandingQueries
*/


std::string cDht::GenerateToken(const cDhtNode& forNode, const cDhtResourceId& infoHash)
{
	// TEMP !!!
	cDhtNodeId token;
	return token.AsString();
}// END GenerateToken



s32 cDht::SendQuery(const cDhtNode& node, cKrpcQuery* query)
{
	query->SendTime(mTimer.ElapsedMs());
	query->SentTo(node);
	mUdpDatagramsSent++;

#if LOG_DHT_MESSAGES
	mPacketLog.LogKrpcMessage(Time(), node.Address(), *query, PTYPE_SEND);
#endif

	return mSocket.Send(query->Message().c_str(), static_cast<u32>(query->Message().size()), &(node.Address()));
}// END SendQuery



void cDht::SendResponse(const net::cSockAddr& to, const cKrpcResponse& response) const
{
#if LOG_DHT_MESSAGES
	mPacketLog.LogKrpcMessage(Time(), to, response, PTYPE_SEND);
#endif

	// TODO : probably want to queue up these messages so we cannot get DOS style attacks
	mSocket.Send(response.Message().c_str(), static_cast<u32>(response.Message().size()), &to);
}// END SendResponse