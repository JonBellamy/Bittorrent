#if USE_CRASH_MONKEY
#include "General/CrashMonkey.h"
#endif

#include <stdio.h>
#include <stdlib.h>

void OnAssert(const char* fn, int ln, const char* message)
{
	Printf("ASSERT\n%s\nLine %d\n%s\n", fn, ln, message);

#if USE_CRASH_MONKEY
	char body[1024];
	sprintf(body, "Assert:\nFile: %s\nLine: %d\n%s\n", fn, ln, message);
	CrashMonkey().PostLog("Assert", body, NULL, true);
#endif

	abort();
}// END OnAssert


