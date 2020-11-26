// Jon Bellamy 21-01-2004
// Encapsulates timer functionality 
// Can be used to time something or count down from a value and tell you when this period has expired


#include "Timer.h"



cTimer::cTimer(eTIMER_TYPE type)
: mTimerType(type)
{
	mTicksPerSecond.QuadPart = 0;
	mIsHighResTimer = QueryPerformanceFrequency(&mTicksPerSecond) == TRUE;
	Reset();
}// END cTimer
	


cTimer::~cTimer(void)
{
}// END ~cTimer



void cTimer::Process(void)
{
	if (mbRunning)
	{
		if (mIsHighResTimer)
		{
			// get the current time 
			QueryPerformanceCounter(&mCurrentTime);

			// update elapsed time 
			mMsElapsed = static_cast<u32> ((mCurrentTime.QuadPart - mStartTime.QuadPart) / (mTicksPerSecond.QuadPart / 1000));

			// keep track of how many ticks have elapsed 
			mTicksElapsed.QuadPart = mCurrentTime.QuadPart - mStartTime.QuadPart;
		}
		else
		{
			u32 currentTime = timeGetTime(); 
			mMsElapsed = currentTime - mLastUpdate;
			mLastUpdate = currentTime;
		}
	}
}// END Process



// resets all timing var's 
void cTimer::Reset(void)
{
	mMsElapsed = 0;

	QueryPerformanceCounter(&mStartTime);		
	mCurrentTime.QuadPart = 0;
	mTicksElapsed.QuadPart = 0;
	
	mCountdownPeriodMs = 0;
	mLastUpdate = timeGetTime();

	mbRepeat = true;
	mbRunning = false;	
}// END Reset



void cTimer::SetCountdownPeriod(u32 period) 
{ 
	if(mTimerType == COUNTDOWN) 
	{
		mMsElapsed = 0;
		mCountdownPeriodMs = period; 
	}
}// END SetCountdownPeriod



// tests if a countdown timer has expired, if the timer is on repeat and has expired then it will reset the values and begin again 
bool cTimer::Expired(void)
{
	if (!mbRunning || 
		mTimerType != COUNTDOWN) 
	{
		return false;
	}

	bool bExpired=false;
	if (mMsElapsed >= mCountdownPeriodMs) 
	{
		bExpired=true;
	}

	// check for a repeating timer 
	if (bExpired && mbRepeat) 
	{
		mbRepeat=true;
		mbRunning=true;
		
		mMsElapsed = 0;

		QueryPerformanceCounter(&mStartTime);		
		mCurrentTime.QuadPart = 0;
		mTicksElapsed.QuadPart = 0;
		mLastUpdate = timeGetTime();
	}

	return bExpired;
}// END Expired



u32 cTimer::ElapsedMs(void) const
{ 
	// performance warning !
	//Process();

	return mMsElapsed; 
}// END ElpasedMs