#include "stdafx.h"
#include "ManagedHelpers.h"

#include <stdio.h>
#include <assert.h>
#include <vcclr.h>



std::string ManagedStringToStlString(String^ managedStr)
{
	// Pin memory so GC can't move it while native function is called
	pin_ptr<const wchar_t> wch = PtrToStringChars(managedStr);

	//printf_s("%S\n", wch);


	// Conversion to char* :
	// Can just convert wchar_t* to char* using one of the
	// conversion functions such as:
	// WideCharToMultiByte()
	// wcstombs_s()
	// ... etc

	size_t convertedChars = 0;
	size_t  sizeInBytes = ((managedStr->Length + 1) * 2);
	errno_t err = 0;
	char ch [1024];

	err = wcstombs_s(&convertedChars,
		ch, sizeInBytes,
		wch, sizeInBytes);

	if (err == 0)
	{
		return std::string(ch);
	}
	//printf_s("wcstombs_s  failed!\n");
	//printf_s("%s\n", ch);
	return "";
}// END ManagedStringToStlString


