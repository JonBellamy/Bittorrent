// Jon Bellamy 02/12/2009
// Handles creation and scheduling of Dht tasks. This is the interface users will deal with


#ifndef DHT_TASK_MANAGER__H_
#define DHT_TASK_MANAGER__H_


#if USE_PCH
#include "stdafx.h"
#endif

#include <vector>

#include "Network/BitTorrent/dht/DhtTask.h"
#include "Network/BitTorrent/dht/Dht.h"



class cDhtTaskManager
{
public:
	cDhtTaskManager();
	~cDhtTaskManager();

private:
	cDhtTaskManager(const cDhtTaskManager& rhs);
	const cDhtTaskManager& operator= (const cDhtTaskManager& rhs);
	bool operator== (const cDhtTaskManager& rhs);


public:
	void StartDht();
	void StopDht();

	void Process();

	typedef enum
	{
		LOW_PRIORITY = 0,
		HIGH_PRIORITY
	}TaskPriority;

	const cDhtFindNodeTask* FindNode(TaskPriority priority, const cDhtNodeId& nodeId, cDhtTask::DhtTaskBehaviourCb behaviourCb, void* behaviourCbParam, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);
	const cDhtGetPeersTask* GetPeers(TaskPriority priority, const cDhtResourceId& infoHash, u32 numNodesToBeginSearch, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);

	// Even though they are not recursive these are still tasks. They must therefore wait until there is time to run them so all outgoing bandwidth is managed
	const cDhtPingTask* PingNode(TaskPriority priority, const net::cSockAddr& pingAddr, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);
	const cDhtAnnounceTask* AnnounceResource(TaskPriority priority, const cDhtResourceId& resourceId, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);

	void OnTaskComplete(const cDhtTask* pTask);

	void DeleteTask(const cDhtTask* pTask);
	void DeleteAllTasks();

	void RestartTask(const cDhtTask* pTask);

	u32	ExpandTaskSearch(const cDhtQueryTask* pTask, u32 extraSearchNodesToAdd);

	TaskPriority GetTaskPriority(const cDhtTask* pTask);
	void SetTaskPriority(const cDhtTask* pTask, TaskPriority priority);

	void SuspendAllTasks();

	// used to suspend / allow task manager to run tasks
	void AllowScheduler(bool b) { mAllowScheduler = b; }
	bool AllowScheduler() const { return mAllowScheduler; }

	cDht& Dht() { return mDht; }
	const cDht& Dht() const { return mDht; }

	bool Running() const { return mStarted; }

private:

	typedef struct
	{
		TaskPriority mPriority;
		cDhtTask* mpTask;

		u32 mRunTime;							// when did this task start to run
		u32 mSuspendTime;						// when did we suspend it
		bool mAutoDelete;

		bool mFlaggedForDelete;
	}ScheduledTask;


	void SuspendTask(ScheduledTask& task);
	void RunTask(ScheduledTask& task);
	void AddTask(ScheduledTask& task);			// new task, wants to run as soon as possible
	
	ScheduledTask* NextTaskToRun();

	bool IsTaskStarved(const ScheduledTask& task) const;
	bool IsTaskWaitingToRun(const ScheduledTask& task) const;

	u32 NumberOfTasksWaitingToRun(bool countHighPriority, bool countLowPriority) const;
	u32 NumberOfRunningTasks(bool countHighPriority, bool countLowPriority) const;

	ScheduledTask* LookupTask(const cDhtTask* pTask);

	enum
	{
		SUSPEND_PERIOD = 32 * 1000,
		LOW_PRIORITY_RUN_TIME = 32 * 1000,
		HIGH_PRIORITY_RUN_TIME = 64 * 1000,

		STARVED_TASK_TIME = SUSPEND_PERIOD * 2,

		MAX_CONCURRENT_TASKS = 4
	};

	cDht mDht;

	bool mStarted;

	typedef std::vector<ScheduledTask> ScheduledTaskVector;
	typedef ScheduledTaskVector::iterator ScheduledTaskVectorIterator;
	ScheduledTaskVector mTasks;

	u32 mLastTaskCreatedTime;

	bool mAllowScheduler;
};




#endif // DHT_TASK_MANAGER__H_