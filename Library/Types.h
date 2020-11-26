// Jon Bellamy 22/11/2005
// typedef's

#define PLATFORM_WIN32 1

#ifdef PLATFORM_WIN32

typedef unsigned char u8;
typedef char s8;

typedef unsigned short u16;
typedef short s16;

typedef int s32;
typedef unsigned int u32;

// M$ specific
typedef __int64 s64;
typedef unsigned __int64 u64;

#include "General/_Assert.h"

#define NULL 0


// TODO : this belongs in a prefix header
#define MEMORY_DEBUGGING 1
#if MEMORY_DEBUGGING

#ifdef _DEBUG
#define _CRTDBG_MAPALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK , __FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif // _DEBUG

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#endif// MEMORY_DEBUGGING


#endif // PLATFORM_WIN32