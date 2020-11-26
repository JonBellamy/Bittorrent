// Jon Bellamy 23/11/2009


#ifndef DHT_TASK__H_
#define DHT_TASK__H_


#if USE_PCH
#include "stdafx.h"
#endif

#include <deque>
#include <vector>
#include <assert.h>

#include "Network/BitTorrent/dht/KrpcMsg.h"
#include "Network/BitTorrent/dht/NodeId.h"
#include "Network/BitTorrent/dht/RoutingTable.h"
#include "Network/BitTorrent/dht/ResourceContactInfo.h"



class cDhtTask
{
public:
	typedef void (*DhtTaskBehaviourCb) (cDhtTask* pTask, DhtNodeVector* pResponseNodes, void* param);
	typedef void (*DhtTaskCompleteCb) (const cDhtTask* pTask, void* param);

	cDhtTask();
	virtual ~cDhtTask();

private:
	cDhtTask(const cDhtTask& rhs);
	const cDhtTask& operator= (const cDhtTask& rhs);
	bool operator== (const cDhtTask& rhs);


public:

	virtual void OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response)=0;	
	virtual void OnSuspend() =0;									// the task manager has suspended us (good time to do house keeping)
	virtual void Restart() =0;
	virtual bool IsTaskComplete() const =0;
	virtual void ProcessJobs() =0;

	virtual void OnTaskResponseTimeout(const cKrpcQuery& sent);		// the Dht has given up on this msg
	virtual void OnTaskComplete();
	virtual void Process() {}

	bool IsRunning() const { return mRunning; }
	void AllowToRun(bool b) { mRunning = b; }

	u32 OutstandingQueries() const { return mOutstandingQueries; }

protected:
	
	bool mRunning;

	DhtTaskCompleteCb mCompleteCb;
	void* mCompleteCbParam;

	u32 mOutstandingQueries;
};



class cDhtQueryTask : public cDhtTask
{
public:
	cDhtQueryTask();

private:
	cDhtQueryTask(const cDhtQueryTask& rhs);
	const cDhtQueryTask& operator= (const cDhtQueryTask& rhs);
	bool operator== (const cDhtQueryTask& rhs);

public:
	virtual void OnTaskResponseTimeout(const cKrpcQuery& sent);		// the Dht has given up on this task

	virtual void TrimUpcomingQueryList() =0;
	virtual u32  InjectExtraSearchQueries(u32 numToAdd) =0;
	virtual bool HasQueriesToSend() const=0;


protected:

	enum
	{
		MAX_OUTSTANDING_QUERIES = 24
	};
};



class cDhtGetPeersTask : public cDhtQueryTask
{
public:
    cDhtGetPeersTask();

private:
	cDhtGetPeersTask(const cDhtGetPeersTask& rhs);
	const cDhtGetPeersTask& operator= (const cDhtGetPeersTask& rhs);
	bool operator== (const cDhtGetPeersTask& rhs);

	
public:

	void GetPeersForResource(const cDhtResourceId& resId, u32 numNodesToStartWith, DhtTaskCompleteCb completeCb, void* completeCbParam);
	void ProcessJobs();
	bool IsTaskComplete() const;
	void TrimUpcomingQueryList() {}
	void Restart();
	u32  InjectExtraSearchQueries(u32 numToAdd);
	void OnSuspend(){}
	void OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response);

	bool HasQueriesToSend() const { return mNodesToQuery.size() > 0; }

	u32 NumberOfPeersFound() const { return static_cast<u32> (mPeersWithResource.size()); }
	const std::vector<net::cSockAddr>& FoundPeersList() const { return mPeersWithResource; }

	const cDhtResourceId& ResourceId() const { return mResourceId; }
	
	u32 SearchHistorySize() const { return mSearchHistory.size(); }

private:

	enum
	{
		HEALTHY_ACTIVE_QUERY_QUEUE_SIZE = 512,		// before we have found the resource, how big can the upcoming search node queue get
		MAX_SEARCH_HISTORY_SIZE = 16384				// if we have search this many nodes, clear the search history (several hundred kb for this size!)
	};

	void SortQueryQueue();
	void DebugPrintQueryQueue() const;

	cDhtResourceId mResourceId;
	u32 mNodeCountToStartWith;

	std::vector<net::cSockAddr> mPeersWithResource;

	std::deque<cDhtNode> mNodesToQuery;
	DhtNodeVector mSearchHistory;

};





class cDhtFindNodeTask : public cDhtQueryTask
{
public:
	cDhtFindNodeTask();

private:
	cDhtFindNodeTask(const cDhtFindNodeTask& rhs);
	const cDhtFindNodeTask& operator= (const cDhtFindNodeTask& rhs);
	bool operator== (const cDhtFindNodeTask& rhs);


public:

	void FindNode(const cDhtNodeId& nodeId, DhtTaskBehaviourCb behaviourCb, void* behaviourCbParam, DhtTaskCompleteCb completeCb, void* completeCbParam);	
	void ProcessJobs();
	bool IsTaskComplete() const;
	void TrimUpcomingQueryList();
	void Restart();
	u32  InjectExtraSearchQueries(u32 numToAdd);
	void OnSuspend();
	void OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response);

	bool HasQueriesToSend() const { return mNodesToQuery.size() > 0; }

	void AddNodeToQuery(const cDhtNode& node) { mNodesToQuery.push_back(node); }
	const cDhtNodeId& SearchTarget() const { return mSearchNodeId; }

private:
	cDhtNodeId mSearchNodeId;
	
	DhtTaskBehaviourCb mBehaviourCb;
	void* mBehaviourCbParam;

	DhtNodeVector mNodesToQuery;
	u32 mNextIndexToQuery;				// nodes below this index have already been queried
};



// basic shell task
class cDhtPingTask : public cDhtTask
{
public:
	cDhtPingTask(const net::cSockAddr& addrToPing, DhtTaskCompleteCb completeCb, void* completeCbParam);
	
	void OnTaskResponseTimeout(const cKrpcQuery& sent);
	void OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response);
	bool IsTaskComplete() const;
	void ProcessJobs();

	void OnSuspend() {}
	void Restart() { assert(0); }

private:
	net::cSockAddr mAddrToPing;
	bool mPingSent;
};



// basic shell task
class cDhtAnnounceTask : public cDhtTask
{
public:
	cDhtAnnounceTask();

	void Announce(const cDhtResourceId& resourceId, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);
	
	bool IsTaskComplete() const;
	void ProcessJobs();
	void Process();
	void OnTaskResponseTimeout(const cKrpcQuery& sent);
	void OnResponse(const cKrpcQuery& sent, const cKrpcResponse& response);
	
	void OnSuspend() {}
	void Restart() { assert(0); }
	
	const cDhtResourceId& ResourceId() const { return mResourceId; }

private:

	enum
	{
		MAX_MSGS_SEND_ONE_SHOT = 4,
		SEND_PERIOD = 250				// Ms
	};

	typedef std::vector<cPeerWithResource> AnnounceList;
	typedef AnnounceList::iterator AnnounceListIterator;
	typedef AnnounceList::const_iterator AnnounceListConstIterator;

	AnnounceList mAnnounceTargets;
	cDhtResourceId mResourceId;			// info hash
	u32 mLastSendTime;
};



#endif // DHT_TASK__H_