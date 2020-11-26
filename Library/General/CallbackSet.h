#ifndef CALLBACKSET_H
#define CALLBACKSET_H


#include <vector>



typedef bool (*NullFilterSig)();


template <class tFilterFn, class tCallbackFn>
class cCallbackSet
{
public:
	typedef struct
	{
		tFilterFn mFilterFn;				// Intended to be used to control if mCb is invoked
		tCallbackFn mCbFn;
		void* mParam;						// Passed to both of the above functions
	}sCallback;


	cCallbackSet()
	: mSafeToDeleteHandler(true)
	{
	}

	void Add(tFilterFn filterFn, tCallbackFn cb, void* pParam);
	bool Remove(tFilterFn filterFn, tCallbackFn cb, void* pParam);
	bool Remove(tCallbackFn cb, void* pParam);
	bool Remove(tCallbackFn cb);
	
	void Clear() { ASSERT(mSafeToDeleteHandler); mCallbacks.clear(); }
	u32 Size() const { return mCallbacks.size(); }
	const sCallback& GetCallback(u32 index) const { return mCallbacks[index]; }
	void SafeToDeleteHandler(bool b) const { mSafeToDeleteHandler = b; }


private:

	typedef std::vector<sCallback> CallbackVector;
	typedef typename CallbackVector::iterator CallbackVectorIterator;
	CallbackVector mCallbacks;

	mutable bool mSafeToDeleteHandler;
};



template <class tFilterFn, class tCallbackFn>
inline void cCallbackSet<tFilterFn, tCallbackFn>::Add(tFilterFn filterFn, tCallbackFn cb, void* pParam)
{
	sCallback callback;
	callback.mFilterFn = filterFn;
	callback.mCbFn = cb;
	callback.mParam = pParam;
	mCallbacks.push_back(callback);
}// END Add



template <class tFilterFn, class tCallbackFn>
inline bool cCallbackSet<tFilterFn, tCallbackFn>::Remove(tFilterFn filterFn, tCallbackFn cb, void* pParam)
{
	ASSERT(mSafeToDeleteHandler);

	if(mCallbacks.empty())
	{
		return false;
	}

	bool removed;
	u32 count = 0;
	do 
	{
		removed = false;

		for(CallbackVectorIterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++)
		{
			sCallback cbSet = *iter;
			if(cbSet.mFilterFn == filterFn &&
			   cbSet.mCbFn == cb && 
			   cbSet.mParam == pParam)
			{
				mCallbacks.erase(iter);
				removed = true;
				count++;
				break;
			}
		}
	} 
	while (removed);

	return count > 0;
}// END Remove



template <class tFilterFn, class tCallbackFn>	
bool cCallbackSet<tFilterFn, tCallbackFn>::Remove(tCallbackFn cb, void* pParam)
{
	ASSERT(mSafeToDeleteHandler);

	bool removed;
	u32 count = 0;
	do 
	{
		removed = false;

		for(CallbackVectorIterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++)
		{
			sCallback cbSet = *iter;
			if( cbSet.mCbFn == cb && 
				cbSet.mParam == pParam)
			{
				mCallbacks.erase(iter);
				removed = true;
				count++;
				break;
			}
		}
	} 
	while (removed);

	return count > 0;
}// END Remove



template <class tFilterFn, class tCallbackFn>
inline bool cCallbackSet<tFilterFn, tCallbackFn>::Remove(tCallbackFn cb)
{
	ASSERT(mSafeToDeleteHandler);

	bool removed;
	u32 count = 0;
	do 
	{
		removed = false;

		for(CallbackVectorIterator iter = mCallbacks.begin(); iter != mCallbacks.end(); iter++)
		{
			sCallback cbSet = *iter;
			if(cbSet.mCbFn == cb)
			{
				mCallbacks.erase(iter);
				removed = true;
				count++;
				break;
			}
		}
	} 
	while (removed);

	return count > 0;
}// END Remove



#endif // CALLBACKSET_H