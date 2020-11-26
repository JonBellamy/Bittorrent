#include "rand.h"

#define _CRT_RAND_S
#include <stdlib.h>
#include <time.h>
#include <assert.h>


// rand_s does not require seeding
/*
void InitRandom()
{
	static bool seeded=false;
	if(!seeded)
	{
		seeded=true;
		srand((unsigned)time(NULL));
	}
}// END InitRandom
*/


u32 RandRange(u32 min, u32 max)
{	
	return min + Rand32(max - min);
}// END RandRange



// returns between 0 & max -1
u32 Rand32(u32 max)
{
	u32 ret;
	u32 r;
	ret = rand_s(&r);
	assert(ret==0);
	return r % max;
}// END Rand32



u16 Rand16(u16 max)
{
	return static_cast<u16> (Rand32(max));
}// END Rand16



// generates a random number between the supplied parameters (remember to seed the generator)
u32 Random(u32 iMin, u32 iMax) 
{	
	return (rand()%(iMax-iMin+1))+iMin;	
}// END Random



// random number between 0 and 1 
float Random_0_1() 
{ 
	u32 ret;
	u32 r;
	ret = rand_s(&r);
	assert(ret==0);
	return r / (float) (UINT_MAX + 1.0); 
}// END Random_0_1



// generates a random float between the passed 2 values
float RandomFloat(float fMin, float fMax)
{ 
	return Random_0_1() * (fMax - fMin) + fMin;
}// END RandomFloat
