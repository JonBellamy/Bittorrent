#ifndef __STRING_UTIL_H_
#define __STRING_UTIL_H_

#include <string>

extern bool StrCmpCaseInsensitive(const std::string& lhs, const std::string& rhs);

extern bool IsCharWhiteSpace(char c);

#endif // __STRING_UTIL_H_