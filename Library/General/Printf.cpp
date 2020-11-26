#include "Printf.h"

#include <stdio.h>
#include <stdarg.h>

bool gSuppressPrintf = false;


static DebugStringOutputCb gCb = NULL;

void SetPrintfHandler(DebugStringOutputCb cb)
{
	gCb = cb;
}// END SetDebugStringOutputCb



extern "C" void Printf(const char* szStr, ...)
{
	va_list args;
	char strPtr[1024];
    
	va_start(args, szStr);
//#if PLATFORM_MACOS == 1
//    vsnprintf(strPtr, 1024, szStr, args);
//#elif PLATFORM_WIN32 == 1
	_vsnprintf( strPtr, 1024, szStr, args );
//#endif
	strPtr[1023] = 0;
    
	// call out
	if(gCb)
	{
		gCb(strPtr);		
	}
  
	if(!gSuppressPrintf)
	{
		printf("%s", strPtr);
	}
}