#ifndef _MLOCK_H
#define _MLOCK_H

// On windows this will be the pthreads wrapper
//#include <pthread.h>


// RAII


class cLock
{
public:
	cLock(/*pthread_mutex_t& mutex,*/ bool autoLock=true)
	: mAutoLock(autoLock)
	//, mMutex(mutex)
	{
		if(mAutoLock)
		{			
			Lock();
		}
	}

	~cLock()
	{
		if(mAutoLock)
		{
			Unlock();
		}
	}

	void Lock()
	{
		//pthread_mutex_lock( &mMutex );
	}

	void Unlock()
	{
		//pthread_mutex_unlock( &mMutex );
	}

protected:
	static void InitLock(/*pthread_mutex_t& mutex*/);

private:
	bool mAutoLock;
	//pthread_mutex_t& mMutex;
};





#endif // _MLOCK_H