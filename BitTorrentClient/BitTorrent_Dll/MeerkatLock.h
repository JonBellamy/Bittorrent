#ifndef MEERKAT_LOCK_H
#define MEERKAT_LOCK_H

#include "General/Lock.h"


// RAII
class cMeerkatLock : public cLock
{
public:
	cMeerkatLock(bool autoLock=true)
	: cLock(mMeerkatMutex, autoLock)
	{
	}

	static void InitLock() { cLock::InitLock(mMeerkatMutex); }

private:
	static pthread_mutex_t mMeerkatMutex;
};




#endif // MEERKAT_LOCK_H