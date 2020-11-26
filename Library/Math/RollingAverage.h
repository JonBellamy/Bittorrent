// Jon Bellamy


#ifndef ROLLING_AVERAGE_H
#define ROLLING_AVERAGE_H




template <class tType, int tSize> 
class cRollingAverage
{
public:	
	cRollingAverage();

	void Clear();
	tType Average() const;
	void Submit(tType n);

private:

	tType mBuf[tSize];
	s32 mNumUsedSlots;
	s32 mNextSlotToWriteTo;

	tType mAverage;
};





//////////////////////////////////////////////////////////////////////////
// inlines


template <class tType, int tSize> 
cRollingAverage<tType, tSize>::cRollingAverage()
: mNumUsedSlots(0)
, mNextSlotToWriteTo(0)
, mAverage(0)
{
	Clear();
}// END cRollingAverage



template <class tType, int tSize> 
inline void cRollingAverage<tType, tSize>::Clear()
{
	for(u32 i=0; i < tSize; i++)
	{
		mBuf[i] = 0;
	}
	mNumUsedSlots = 0;
	mNextSlotToWriteTo = 0;
	mAverage = 0;
}// END Clear



template <class tType, int tSize> 
inline tType cRollingAverage<tType, tSize>::Average() const
{
	return mAverage;
}// END Average



template <class tType, int tSize> 
inline void cRollingAverage<tType, tSize>::Submit(tType n)
{
	mBuf[mNextSlotToWriteTo] = n;
		
	if(mNumUsedSlots < tSize)
	{
		mNumUsedSlots++;
	}	

	mNextSlotToWriteTo++;	
	if(mNextSlotToWriteTo >= tSize)
	{
		mNextSlotToWriteTo = 0;
	}	


	// cache the average
	if(mNumUsedSlots == 0)
	{
		mAverage = 0;
	}

	tType average=0;
	for(s32 i=0; i < mNumUsedSlots; i++)
	{
		average += mBuf[i];
	}
	mAverage = average / mNumUsedSlots;
}// END Submit




#endif // ROLLING_AVERAGE_H
