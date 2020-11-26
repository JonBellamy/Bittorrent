
#include <assert.h>
#include "StringUtil.h"


bool StrCmpCaseInsensitive(const std::string& lhs, const std::string& rhs)
{
#if defined PLATFORM_MACOS || defined PLATFORM_ANDRIOD
	return (strcasecmp(lhs.c_str(), rhs.c_str()) == 0);
#elif defined PLATFORM_WIN32
	return (_stricmp(lhs.c_str(), rhs.c_str()) == 0);
#else
	assert(0);
#endif
}// END StrCmpCaseInsensitive



#define TAB 9
#define SPACE_BAR 32
bool IsCharWhiteSpace(char c)
{
	return (c == TAB || c == SPACE_BAR || c == '\n' || c == '\r');
}