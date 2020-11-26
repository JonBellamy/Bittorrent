#ifndef __ASSERT___H
#define __ASSERT___H

#ifndef NDEBUG
//#include <cassert>

#define ASSERT(condition)																\
   {																					\
   if(!(condition))																		\
	  {																					\
		OnAssert(__FILE__, __LINE__, NULL);												\
	  }																					\
   }

#define ASSERT_MSG(condition, message)													\
   {																					\
   if(!(condition))																		\
	  {																					\
	    OnAssert(__FILE__, __LINE__, message);											\
	  }																					\
   }
#else
#define ASSERT(condition) (condition)
#define ASSERT_MSG(condition, message) (condition, message)
#endif

extern void OnAssert(const char* fn, int ln, const char* message);

#endif // __ASSERT___H
