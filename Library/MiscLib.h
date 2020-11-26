// Jon Bellamy
// 28/04/2004
// Helper Function Lib 


#ifndef MISC_H
#define MISC_H

#include "stdafx.h"
#include "FileHelpers.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// Macros

// macro to read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)


namespace MiscLib
{


	//////////////////////////////////////////////////////////////////////////
	// Typedef's / Structs 

	typedef struct tTimeStruct
	{
		int iHours;
		int iMinutes;
		int iSeconds;
		int iMilleSeconds;
	}TimeStruct;



	// **********
	// Prototypes 
	// **********


	//////////////////////////////////////////////////////////////////////////
	// String Helpers


	// 'c:\\myfiles\\hello.txt' returns 'hello.txt'
	char* ExtractFileNameFromFullPath( char *szFullFileName );

	// 'c:\\myfiles\\hello.txt' returns 'txt'
	char* ExtractFileExtension( char *szFullFileName );

	// extracts the substring starting after the first occurrence of the passed char and ending before the last occurrence of the second passed char
	//bool ExtractString( char *szFileName, char *szOut, char cStartDelimiter, char cEndDelimiter );

	// compares two strings ignoring case
	int StrCmpCI( const char *szStr1, const char *szStr2 );


	//////////////////////////////////////////////////////////////////////////
	// Time Values 

	// returns the passed number of milliseconds as hh:mm:ss.msmsms
	TimeStruct ConvertMsToTime( int iMs );

	// returns the number of milliseconds from the passed time struct 
	int ConvertTimeToMs( TimeStruct *pTS );

	// builds a time struct from the passed string 
	bool StringToTimeStruct ( char *szTimeString, TimeStruct *pTS );



}// END namespace MiscLib


#endif // Misc.h