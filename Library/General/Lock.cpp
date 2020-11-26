#include "Lock.h"



void cLock::InitLock(pthread_mutex_t& mutex)
{
	int ret;
	pthread_mutexattr_t    attr;
	
	ret = pthread_mutexattr_init(&attr);
	ASSERT(ret == 0);
	
	ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	ASSERT(ret == 0);

	ret = pthread_mutex_init (&mutex, &attr);
	ASSERT(ret == 0);

	ret = pthread_mutexattr_destroy(&attr);
	ASSERT(ret == 0);
}

