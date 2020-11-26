// Jon Bellamy 18/11/2009
// http://bittorrent.org/beps/bep_0005.html

#ifndef DHT__H_
#define DHT__H_


#if USE_PCH
#include "stdafx.h"
#endif


#include <list>

#include "Network/UdpSocket.h"
#include "Network/BitTorrent/dht/DhtTask.h"
#include "Network/BitTorrent/dht/KrpcMsg.h"
#include "Network/BitTorrent/dht/RoutingTable.h"
#include "Network/BitTorrent/dht/ResourceContactInfo.h"
#include "General/Timer.h"


#define LOG_DHT_MESSAGES 0

#if LOG_DHT_MESSAGES
#include "Network/BitTorrent/dht/DhtPacketLog.h"
#endif


class cDht
{
public:
    cDht();

private:
	cDht(const cDht& rhs);
	const cDht& operator= (const cDht& rhs);
	bool operator== (const cDht& rhs);

	
public:

	void Init();
	void DeInit();

	void Process();

	void OnQueryResponseReceived(const cKrpcQuery& query);
	const cKrpcQuery* GetOutstandingMessage(u16 transactionId);
	

	bool PingNode(const cDhtNode& node, cDhtTask* pTask);
	bool PingNode(const net::cSockAddr& addr, cDhtTask* pTask);
	bool FindNode(const cDhtNode& node, const cDhtNodeId& target, cDhtTask* pTask);
	bool GetPeers(const cDhtNode& node, const cDhtResourceId& infoHash, cDhtTask* pTask);
	bool AnnouncePeer(const cDhtNode& to, const cDhtResourceId& infoHash, u16 port, const std::string& token, cDhtTask* pTask);

	const cDhtNodeId& LocalNodeId() const { return mLocalNode.Id(); }
	cDhtRoutingTable& RoutingTable() { return mRoutingTable; }

	u32 Time() const { return mTimer.ElapsedMs(); }

	void OnUnMatchedResponseReceived() { mUdpUnmatchedResponsesRcvd++; }
	void OnGetPeersResponse() { mGetPeersMsgsResponseRcvd++; }

	std::string GenerateToken(const cDhtNode& forNode, const cDhtResourceId& infoHash);

	void SendResponse(const net::cSockAddr& to, const cKrpcResponse& response) const;

	enum
	{
		COMPACT_PEER_REPRESENTATION_SIZE = 6,
		COMPACT_NODE_REPRESENTATION_SIZE = cDhtNodeId::NODE_ID_SIZE + COMPACT_PEER_REPRESENTATION_SIZE,		// 26

		NODE_COUNT_FOR_QUERY_AND_RETURN = 8,

		DEFAULT_DHT_PORT = 6881,

		RECV_BUFFER_SIZE = 2*1024,
		SOCK_RECV_BUFFER_SIZE = 512*1024,
		SOCK_SEND_BUFFER_SIZE = 512*1024,

		// This is a tempory measure (the value SHOULD be 30 seconds) while the debug build is slow and the dht is starving to death
#if 0
		SENT_QUERY_TIMEOUT = 75 * 1000
#else
		SENT_QUERY_TIMEOUT = 30 * 1000
#endif

	};

#if LOG_DHT_MESSAGES
	cDhtPacketLog& PacketLog() { return mPacketLog; }
#endif

private:

	void LoadBootStrapNodes(const char* bootStrapFilename);

	s32 SendQuery(const cDhtNode& node, cKrpcQuery* query);

	cTimer mTimer;

	mutable net::cUdpSocket mSocket;
	u8 mRecvBuffer[RECV_BUFFER_SIZE];
	
	u32 mUdpDatagramsSent;
	u32 mUdpDatagramsNoReply;
	u32 mUdpUnmatchedResponsesRcvd;
	u32 mGetPeersMsgsSent;
	u32 mGetPeersMsgsResponseRcvd;

	cDhtNode mLocalNode;

	// used to find nodes 'close' to us
	const cDhtFindNodeTask* mpBuildRoutingTableTask;
	static void BuildRoutingTableTaskCompleteCb(const cDhtTask* pTask, void* param);




	// messages we are still awaiting a response to
	typedef std::list<const cKrpcQuery*> OutstandingQueryList;
	typedef OutstandingQueryList::iterator OutstandingQueryList_Iterator;
	typedef OutstandingQueryList::const_iterator OutstandingQueryList_ConstIterator;

	OutstandingQueryList mOutstandingQueries;


	cDhtRoutingTable mRoutingTable;

#if LOG_DHT_MESSAGES
	mutable cDhtPacketLog mPacketLog;
#endif
};




#endif // DHT__H_