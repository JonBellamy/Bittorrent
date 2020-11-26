// Jon Bellamy 02/12/2009
// Handles creation and scheduling of Dht tasks. This is the interface users will deal with


#include "DhtTaskManager.h"

#include <assert.h>

#include "Network/BitTorrent/dht/Dht.h"
#include "General/Rand.h"


using namespace net;


// debug flag to keep a small number of tasks running constantly
#define FEW_TASKS_JUST_RUN 0



cDhtTaskManager::cDhtTaskManager()
: mStarted(false)
, mAllowScheduler(true)
, mLastTaskCreatedTime(0)
{
}// END cDhtTaskManager



cDhtTaskManager::~cDhtTaskManager()
{
}// END ~cDhtTaskManager



void cDhtTaskManager::StartDht()
{
	Printf("Starting Dht\n");
	mStarted = true;
	mAllowScheduler = true;
	mLastTaskCreatedTime = 0;
	Dht().Init();	
}// END StartDht



void cDhtTaskManager::StopDht()
{
	Printf("Stopping Dht\n");
	assert(mStarted);
	mStarted = false;

	Dht().DeInit();
	DeleteAllTasks();
	mLastTaskCreatedTime = 0;
}// END StopDht



void cDhtTaskManager::Process()
{
	if(!Running())
	{
		return;
	}

	Dht().Process();

	// delete any tasks that are flagged
	for(u32 i=0; i < mTasks.size(); i++)
	{
		ScheduledTask& t = mTasks[i];
		if(t.mFlaggedForDelete)
		{
			assert(t.mpTask->IsTaskComplete() && t.mpTask->OutstandingQueries() == 0);
			DeleteTask(t.mpTask);
			break;
		}
	}

	if(AllowScheduler())
	{
		for(u32 i=0; i < mTasks.size(); i++)
		{
			ScheduledTask& t = mTasks[i];

			// nothing much done in task process, lets keep it that way
			t.mpTask->Process();

			// When scheduling i'm not counting bytes or accumulating time slices etc, a task is simply alloted a set number of seconds to run before it 
			// will be suspended for a set period. While suspended it will still process incoming responses it has outstanding.

			// suspend task as required ...
			if(t.mpTask->IsRunning())
			{
				u32 runtime = Dht().Time() - t.mRunTime;
				bool suspend;

				// check for exhausted tasks
				if(t.mpTask->IsTaskComplete())
				{
					suspend = true;
				}
				else
				{
					// time up?
					switch(t.mPriority)
					{
					case LOW_PRIORITY:
						suspend = runtime >= LOW_PRIORITY_RUN_TIME;
						break;
					case HIGH_PRIORITY:
						suspend = runtime >= HIGH_PRIORITY_RUN_TIME;
						break;

					default:
						assert(0);
					}
				}

#if FEW_TASKS_JUST_RUN
				if(mTasks.size() < MAX_CONCURRENT_TASKS)
				{
					suspend=false;
				}
#endif

				if(suspend)
				{
					SuspendTask(t);
				}
			}
		}



		// Run tasks as required ...
		u32 tasksRunning = NumberOfRunningTasks(true, true);
		u32 tasksWaitingToRun = NumberOfTasksWaitingToRun(true, true);
		if(tasksRunning < MAX_CONCURRENT_TASKS &&
		   tasksWaitingToRun > 0)
		{
			ScheduledTask* pNextTask = NextTaskToRun();
			assert(pNextTask);
			RunTask(*pNextTask);
		}
	}
}// END Process



void cDhtTaskManager::OnTaskComplete(const cDhtTask* pTask)
{
	ScheduledTask* task = LookupTask(pTask);
	if(!task)
	{
		assert(0);
		return;
	}

	// TODO : This probably wants to just flag for delete as this will obviously invalidate all iterators.
	//        I think its ok for now as there aren't any iterators further down the call stack from here.
	if(task->mAutoDelete)
	{
		task->mFlaggedForDelete=true;		
	}
}// END OnTaskComplete



void cDhtTaskManager::DeleteTask(const cDhtTask* pTask)
{
//	assert(pTask->IsTaskComplete());

	// TODO: if not complete ...
		// mark for delete
		// wait to drain
		// check with dht that no outstanding msgs relate to the task


	// remove
	for(ScheduledTaskVectorIterator iter=mTasks.begin(); iter != mTasks.end(); iter++)
	{
		const ScheduledTask& t = *iter;
		if(t.mpTask == pTask)
		{
			delete t.mpTask;
			mTasks.erase(iter);
			return;
		}
	}
	assert(0);
}// END DeleteTask



void cDhtTaskManager::DeleteAllTasks()
{
	while(mTasks.empty() == false)
	{
		ScheduledTaskVectorIterator iter = mTasks.begin();
		ScheduledTask& t = *iter;
		delete t.mpTask;
		mTasks.erase(iter);
	}
}// END DeleteAllTasks



void cDhtTaskManager::RestartTask(const cDhtTask* pTask)
{
	Printf("DHT: cDhtTaskManager::RestartTask %IX\n", pTask);

	ScheduledTask* task = LookupTask(pTask);
	task->mpTask->Restart();
	AddTask(*task);
}// END RestartTask



u32	cDhtTaskManager::ExpandTaskSearch(const cDhtQueryTask* pTask, u32 extraSearchNodesToAdd)
{
	if(dynamic_cast<const cDhtQueryTask*> (pTask) == NULL)
	{
		assert(0);
		return 0;
	}
	Printf("DHT: Expanding task %IX search by %u nodes.\n", pTask, extraSearchNodesToAdd);
	ScheduledTask* pScheduledTask = LookupTask(pTask);
	u32 numQueriesAdded = static_cast<cDhtQueryTask*> (pScheduledTask->mpTask)->InjectExtraSearchQueries(extraSearchNodesToAdd);
	static_cast<cDhtQueryTask*> (pScheduledTask->mpTask)->ProcessJobs();
	return numQueriesAdded;
}// END ExpandTaskSearch



cDhtTaskManager::TaskPriority cDhtTaskManager::GetTaskPriority(const cDhtTask* pTask)
{
	return LookupTask(pTask)->mPriority;
}// END GetTaskPriority



void cDhtTaskManager::SetTaskPriority(const cDhtTask* pTask, TaskPriority priority)
{
	LookupTask(pTask)->mPriority = priority;
}// END SetTaskPriority



void cDhtTaskManager::SuspendAllTasks()
{
	for(u32 i=0; i < mTasks.size(); i++)
	{
		ScheduledTask& t = mTasks[i];
		SuspendTask(t);
	}
}// END SuspendAllTasks



void cDhtTaskManager::SuspendTask(ScheduledTask& task)
{
	Printf("DHT: cDhtTaskManager::SuspendTask %IX\n", task.mpTask);

	task.mRunTime = 0;
	task.mSuspendTime = Dht().Time();
	task.mpTask->AllowToRun(false);

	task.mpTask->OnSuspend();
}// END SuspendTask



void cDhtTaskManager::RunTask(ScheduledTask& task)
{
	Printf("DHT: cDhtTaskManager::RunTask %IX\n", task.mpTask);

	// don't run tasks that have no work left!
	assert(task.mpTask->IsTaskComplete() == false);

	task.mRunTime = Dht().Time();
	task.mSuspendTime = 0;
	task.mpTask->AllowToRun(true);
	task.mpTask->ProcessJobs();
}// END RunTask



// new task, wants to run as soon as possible.
void cDhtTaskManager::AddTask(ScheduledTask& task)	
{
	task.mRunTime = 0;

	u32 randWait=0;
	// if we are adding lots of tasks at once, stagger them slightly
	if((Dht().Time() - mLastTaskCreatedTime) < 5000)
	{
		randWait += Rand32(SUSPEND_PERIOD);
	}

	// Set its suspend time so that the scheduler picks it up next
	task.mSuspendTime = (Dht().Time() - SUSPEND_PERIOD) + randWait;
	task.mpTask->AllowToRun(false);
}// END AddTask



bool cDhtTaskManager::IsTaskStarved(const ScheduledTask& task) const
{
	return (task.mpTask->IsRunning()==false && ((Dht().Time() - task.mSuspendTime) >= STARVED_TASK_TIME));
}// END IsTaskStarved



bool cDhtTaskManager::IsTaskWaitingToRun(const ScheduledTask& task) const
{
	if(task.mpTask->IsRunning())
	{
		return false;
	}
	return (Dht().Time() - task.mSuspendTime >= SUSPEND_PERIOD);
}// END IsTaskWaitingToRun



u32 cDhtTaskManager::NumberOfTasksWaitingToRun(bool countHighPriority, bool countLowPriority) const
{
	u32 count=0;
	for(u32 i=0; i < mTasks.size(); i++)
	{
		const ScheduledTask& t = mTasks[i];

		if( t.mpTask->IsRunning() == false && 
			t.mpTask->IsTaskComplete() == false &&
			Dht().Time() - t.mSuspendTime >= SUSPEND_PERIOD)
		{
			if(countHighPriority && t.mPriority == HIGH_PRIORITY)
			{
				count++;
			}
			else if(countLowPriority && t.mPriority == LOW_PRIORITY)
			{
				count++;
			}
		}
	}
	return count;
}// END NumberOfTasksWaitingToRun



u32 cDhtTaskManager::NumberOfRunningTasks(bool countHighPriority, bool countLowPriority) const
{
	u32 count=0;
	for(u32 i=0; i < mTasks.size(); i++)
	{
		const ScheduledTask& t = mTasks[i];

		if(t.mpTask->IsRunning())
		{
			if(countHighPriority && t.mPriority == HIGH_PRIORITY)
			{
				count++;
			}
			else if(countLowPriority && t.mPriority == LOW_PRIORITY)
			{
				count++;
			}
		}
	}
	return count;
}// END NumberOfRunningTasks



cDhtTaskManager::ScheduledTask* cDhtTaskManager::NextTaskToRun()
{
	if(NumberOfTasksWaitingToRun(true, true) == 0)
	{
		return NULL;
	}

	ScheduledTask* pTask=NULL;
	u32 highSuspendTime=0;

	for(u32 i=0; i < mTasks.size(); i++)
	{
		ScheduledTask& t = mTasks[i];

		if(!IsTaskWaitingToRun(t) || t.mpTask->IsTaskComplete())
		{
			continue;
		}

		if(t.mSuspendTime > highSuspendTime)
		{
			highSuspendTime = t.mSuspendTime;
			pTask = &t;
		}
	}

	return pTask;
}// END NextTaskToRun



cDhtTaskManager::ScheduledTask* cDhtTaskManager::LookupTask(const cDhtTask* pTask)
{
	for(u32 i=0; i < mTasks.size(); i++)
	{
		ScheduledTask& t = mTasks[i];
		if(t.mpTask == pTask)
		{
			return &t;
		}
	}
	assert(0);
	return NULL;
}// END LookupTask




//////////////////////////////////////////////////////////////////////////
// add tasks ...


// Even though they are not recursive these are still tasks. They must therefore wait until there is time to run them so all outgoing bandwidth is managed
//const cDhtPingTask*  PingNode(const cDhtNodeId& nodeId, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);
//const cDhtAnnounceTask*  AnnounceResource(const cDhtResourceId& resourceId, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam);


const cDhtFindNodeTask* cDhtTaskManager::FindNode(TaskPriority priority, const cDhtNodeId& nodeId, cDhtTask::DhtTaskBehaviourCb behaviourCb, void* behaviourCbParam, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	cDhtFindNodeTask* pTask = new cDhtFindNodeTask;
	mLastTaskCreatedTime = Dht().Time();
	
	ScheduledTask t;
	t.mPriority = priority;
	t.mpTask = pTask;
	t.mAutoDelete = false;
	t.mFlaggedForDelete = false;
	AddTask(t);

	mTasks.push_back(t);

	pTask->FindNode(nodeId, behaviourCb, behaviourCbParam, completeCb, completeCbParam);

	return pTask;
}// END FindNode



const cDhtGetPeersTask* cDhtTaskManager::GetPeers(TaskPriority priority, const cDhtResourceId& infoHash, u32 numNodesToBeginSearch, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	cDhtGetPeersTask* pTask = new cDhtGetPeersTask;
	mLastTaskCreatedTime = Dht().Time();

	ScheduledTask t;
	t.mPriority = priority;
	t.mpTask = pTask;
	t.mAutoDelete = false;
	t.mFlaggedForDelete = false;
	AddTask(t);

	mTasks.push_back(t);

	pTask->GetPeersForResource(infoHash, numNodesToBeginSearch, completeCb, completeCbParam);

	return pTask;
}// END FindNode



const cDhtPingTask* cDhtTaskManager::PingNode(TaskPriority priority, const cSockAddr& pingAddr, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	cDhtPingTask* pTask = new cDhtPingTask(pingAddr, completeCb, completeCbParam);
	mLastTaskCreatedTime = Dht().Time();

	ScheduledTask t;
	t.mPriority = priority;
	t.mpTask = pTask;
	t.mAutoDelete = true;
	t.mFlaggedForDelete = false;
	AddTask(t);
	mTasks.push_back(t);

	// note the ping isn't sent here, instead it waits for the scheduler

	return pTask;
}// END PingNode



const cDhtAnnounceTask* cDhtTaskManager::AnnounceResource(TaskPriority priority, const cDhtResourceId& resourceId, cDhtTask::DhtTaskCompleteCb completeCb, void* completeCbParam)
{
	cDhtAnnounceTask* pTask = new cDhtAnnounceTask();
	mLastTaskCreatedTime = Dht().Time();
	pTask->Announce(resourceId, completeCb, completeCbParam);

	ScheduledTask t;
	t.mPriority = priority;
	t.mpTask = pTask;
	t.mAutoDelete = true;
	t.mFlaggedForDelete = false;
	AddTask(t);
	mTasks.push_back(t);

	return pTask;
}// END AnnounceResource