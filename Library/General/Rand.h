#ifndef _RAND_H
#define _RAND_H



//extern void InitRandom();

// all return up to max -1
extern u32 RandRange(u32 min, u32 max);
extern u32 Rand32(u32 max=0xFFFFFFFF);
extern u16 Rand16(u16 max=0xFFFF);
extern float Random_0_1();
extern float RandFloat(float fMax);


#endif // _RAND_H