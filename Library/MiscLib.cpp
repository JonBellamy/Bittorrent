// Jon Bellamy
// 28/04/2004

// Helper Function Lib 

#include "stdafx.h"
#include "MiscLib.h"

#include <Windows.h>			// DANGER shouldn't really be including this with windows forms 

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <assert.h>



//////////////////////////////////////////////////////////////////////////
// String Helpers


char* MiscLib::ExtractFileNameFromFullPath( char *szFullFileName )
{
	char *szFileName;
	szFileName = strrchr(szFullFileName, '\\');
	szFileName++;

	return szFileName;
}// END ExtractFileNameFromFullPath



// 'c:\\myfiles\\hello.txt' returns 'txt'
char* MiscLib::ExtractFileExtension( char *szFullFileName )
{
	char *szFileName;
	szFileName = strrchr(szFullFileName, '.');
	szFileName++;

	return szFileName;
}// END ExtractFileExtension


/*
// extracts the substring starting after the first occurrence of the passed char and ending before the last occurrence of the second passed char
bool MiscLib::ExtractString( char *szFileName, char *szOut, char cStartDelimiter, char cEndDelimiter )
{
	char *szStart, *szEnd;
	char szBuf[512];
	int i=0;

	// find the first character 
	szStart = strchr(szFileName, cStartDelimiter);
	if (szStart == NULL) 
	{
		strcpy_s(szBuf, szFileName, 512);
		return false;
	}

	// find the last character 
	szEnd = strrchr(szFileName, cEndDelimiter);
	if (szEnd == NULL) 
	{
		strcpy_s(szBuf, szFileName, 512);
		return false;
	}

	szStart++;

	// underscore delimits the level number 
	while (szStart != szEnd)
	{
		szBuf[i] = *szStart;

		i++;
		szStart++;
	}

	szBuf[i] = NULL;

	// return the string
	strcpy(szOut, szBuf);

	return true;
}// END ExtractString
*/


// compares two strings ignoring case
int MiscLib::StrCmpCI( const char *szStr1, const char *szStr2 )
{
	char szLower1[256], szLower2[256];
	unsigned int i;

	strcpy_s(szLower1, 256, szStr1);
	strcpy_s(szLower2, 256, szStr2);

	for (i=0; i<strlen(szStr1); i++) szLower1[i] = tolower(szLower1[i]);
	for (i=0; i<strlen(szStr2); i++) szLower2[i] = tolower(szLower2[i]);

	return strcmp(szLower1, szLower2);
}// END StrCmpCI



// returns the passed number of milleseconds as hh:mm:ss:msmsms
MiscLib::TimeStruct MiscLib::ConvertMsToTime( int iMs )
{
	int iMsTmp;
	TimeStruct time;

	iMsTmp = iMs;

	// how many hours 
	time.iHours = (int) iMsTmp / (1000 * 60 * 60);
	iMsTmp -= time.iHours * (1000 * 60 * 60);

	// minutes
	time.iMinutes = (int) iMsTmp / (1000 * 60);
	iMsTmp -= time.iMinutes * (1000 * 60);

	// seconds
	time.iSeconds = (int) iMsTmp / (1000);
	iMsTmp -= time.iSeconds * (1000);

	// Ms
	time.iMilleSeconds = (int) iMsTmp;
	return time;
}// END ConvertMsToTime



// returns the number of milleseconds from the passed time struct 
int MiscLib::ConvertTimeToMs( TimeStruct *pTS )
{
	int iMS=0;

	iMS += pTS->iHours * ((60*60)*1000);
	iMS += pTS->iMinutes * (60*1000);
	iMS += pTS->iSeconds * 1000;
	iMS += pTS->iMilleSeconds;

	return iMS;
}// END ConvertTimeToMs



// builds a time struct from the passed string 
bool MiscLib::StringToTimeStruct ( char *szTimeString, TimeStruct *pTS )
{
	char szTimeStr[32];
	char *szH, *szM, *szS, *szMS;
	int iH, iM, iS, iMS;

	strcpy_s(szTimeStr, 32, szTimeString);

//	szH = strchr(szTimeStr, ':');
	szH = szTimeStr;
	if (!szH) return false;

	szM = strchr(szH, ':');
	if (!szM) return false;
	szM++;
	
	szS = strchr(szM, ':');
	if (!szS) return false;
	szS++;
	
	szMS = strchr(szS, '.');
	if (!szMS) return false;
	szMS++;
	
	char *c;
	c = strchr(szH, ':');
	*c=NULL;
	c = strchr(szM, ':');
	*c=NULL;
	c = strchr(szS, '.');
	*c=NULL;

	pTS->iHours = iH = atoi(szH);
	pTS->iMinutes = iM = atoi(szM);
	pTS->iSeconds = iS = atoi(szS);
	pTS->iMilleSeconds = iMS = atoi(szMS);

	return true;
}// END StringToTimeStruct 


