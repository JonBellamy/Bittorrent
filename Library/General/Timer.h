// Jon Bellamy 21-01-2004
// Encapsulates timer functionality 
// Can be used to either time something or can count down from a value and tell you when this period has expired


#ifndef __TIMER_H_
#define __TIMER_H_


#include <windows.h>

#if USE_PCH
#include "stdafx.h"
#endif

#include <mmsystem.h>



class cTimer
{
public:

	typedef enum
	{
		STOPWATCH=0,		// a standard stop watch timer 
		COUNTDOWN			// a timer that counts down from a preset value then expires 
	} eTIMER_TYPE;


	cTimer(eTIMER_TYPE type);
	~cTimer(void);


	void Process(void);

	void Reset(void);


	// tests if a countdown timer has expired 
	bool Expired(void);

	u32 ElapsedMs(void) const;


	//////////////////////////////////////////////////////////////////////////
	// Accessors

	eTIMER_TYPE GetType(void) { return mTimerType; }

	void SetRepeat(bool bRepeat) { mbRepeat = bRepeat; } 
	bool GetRepeat(void) { return mbRepeat; } 

	void SetCountdownPeriod(u32 period);

	bool GetRunning(void) { return mbRunning; }
	void Start(void) { mbRunning = true; }
	void Stop(void) { mbRunning = false; }

	// used for very high res timing functions 
	s64 GetElapsedTicks(void) { return mTicksElapsed.QuadPart; }
	s64 GetTicksPerSecond(void) { return mTicksPerSecond.QuadPart; }


private:

	bool mIsHighResTimer;				// is the timer high resolution
	eTIMER_TYPE mTimerType;				// the type of timer 
	bool mbRepeat;						// does the timer reset and count again when it finishes 
	bool mbRunning;						// is the timer currently running 
	u32	mCountdownPeriodMs;				// Ms which must elapse to make the timer expire (eg 60 means after 60Ms the timer will return expired) 	
	u32 mMsElapsed;						// Ms elapsed since timer started 


	//////////////////////////////////////////////////////////////////////////
	// High resolution timer var's 

	
	LARGE_INTEGER	mTicksPerSecond;	// how many times the high res timer ticks in a second 
	LARGE_INTEGER	mStartTime;			// number of elapsed ticks since this timer Init() was called 
	LARGE_INTEGER	mCurrentTime;
	LARGE_INTEGER	mTicksElapsed;		// total number of ticks elapsed since timer was started (used for very high res timing)	


	//////////////////////////////////////////////////////////////////////////
	// Non high res timer var's 

	u32	mLastUpdate;					// used to calculate timing values 
};


#endif // __TIMER_H_