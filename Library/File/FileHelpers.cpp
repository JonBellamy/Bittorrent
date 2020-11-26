#if USE_PCH
#include "stdafx.h"
#endif
#include "FileHelpers.h"

#undef UNICODE					// more DANGER, this will grab the ascii flavours of the file io functions

#include <Windows.h>			// DANGER shouldn't really be including this with windows forms 

#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <assert.h>



namespace FileHelpers
{

// tests if a file exists, does not alter or open the file 
bool FileExists( const char *szFileName )
{
	FILE *f;

	if (strcmp(szFileName, "") ==0)
		return false;

	f = fopen(szFileName, "r");
	if ( f == NULL ) return false;
	else
	{
		fclose(f);
		return true;
	}
}// END FileExists



// the passed file pointer must be open, it will not be closed and the file pointer will be returned to the begining 
s32 GetFileSize( FILE *pFile )
{
	int iSize;

	fseek(pFile, 0, SEEK_END);

	iSize = ftell(pFile);

	fseek(pFile, 0, SEEK_SET);

	return iSize;
}// END GetFileSize



s32  GetFileSize(const char* fn )
{
	FILE* f = fopen(fn, "rb");
	s32 size = GetFileSize(f);
	fclose(f);
	return size;
}// END GetFileSize



bool IsDirectory( const char	*fileName )
{
	WIN32_FIND_DATA	findD;
	bool			rc;
	HANDLE			nextSrch;	
	
	rc = false;
	
	nextSrch = FindFirstFile( fileName, &findD );
	
	if( nextSrch != INVALID_HANDLE_VALUE )
	{
		if( findD.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			rc = true;
		}
	}
	
	return( rc );
}// END IsDirectory



bool MkDir( const char	*fileName )
{
	bool	rc;
	INT		result;
	
	rc = false;
	
	result = _mkdir( fileName );
	
	return rc == 0;
}// END MkDir



static char* SlashTest( char* inStr )
{
	char	*srch;
	
	srch = strchr( inStr, '/' );
	if( !srch )
	{
		srch = strchr( inStr, '\\' );
	}
	
	return( srch );
}// END SlashTest



// ensures that all the folders in the passed file name exist and creates any that don't
bool MakeAllDirs( const char	*fileName )
{
	char szBuf[1024];
	strcpy_s(szBuf, 1024, fileName);

	char	*workingName, *srch, saveChar;
	bool	rc;

	workingName = szBuf;

	rc = true;

	//while there is a slash left in the working name from left to right
	while( rc && ((srch = SlashTest( workingName )) != NULL ))
	{
		// substitute the slash with the terminator
		saveChar = srch[ 0 ];
		srch[ 0 ] = 0;

		//test whether this exists
		if( !IsDirectory( szBuf ))
		{
			//it doesn't?

			// don't try and create a drive letter 
			//if(szBuf[1] != ':') 
			{
				//Make it
				rc = MkDir( szBuf );
			}
		}

		//replace the slash
		srch[ 0 ] = saveChar;

		//advance the working name to the right of the slash
		workingName = &srch[ 1 ];
	}

	return( rc );
}// END MakeAllDirs



// recursive function that scans the passed folder and all sub folders for the given file extension
void ScanForFileTypes( const char *szSearchFolder, char *szExtension, CBFileFind cbFunc, void *pParam)
{
	char	szDir[MAX_PATH], szFullFileName[MAX_PATH];
	char	ext[64], *c;
	unsigned int i;
	HANDLE	dirHandle;
	WIN32_FIND_DATA fileFindData;
	
	
	char szRestoreDir[MAX_PATH];
	_getcwd(szRestoreDir, MAX_PATH);

	_chdir(szSearchFolder);
	
	// scan everyting 
	dirHandle = FindFirstFile( "*", &fileFindData );
	
	while( true )
	{
		// end of this folder 
		if( dirHandle == INVALID_HANDLE_VALUE )
		{
			break;
		}
		
		// new folder, recurse into it 
		else if( fileFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			if ( (strcmp(".", fileFindData.cFileName) != 0) && 
				(strcmp("..", fileFindData.cFileName) != 0) )
			{
				// save the current folder name 
				_getcwd(szDir, MAX_PATH);
				
				ScanForFileTypes(fileFindData.cFileName, szExtension, cbFunc, pParam);
				
				// restore the previous current folder 
				_chdir(szDir);
			}
		}
		
		else if ( (strcmp(".", fileFindData.cFileName) != 0) && 
			(strcmp("..", fileFindData.cFileName) != 0) )
		{
			// check that there is an extension 
			c = strrchr(fileFindData.cFileName, '.');
			
			// we have a file not a folder, extract the extension and test against the target 
			if ( (c != NULL) && (strlen(c) >= 1) ) 
			{
				strcpy(ext, c+1);
				
				// ensure all chars are lower case
				for (i=0; i < strlen(ext); i++)
				{
					ext[i] = tolower(ext[i]);
				}
				
				// check for match and wildcard, if found than call the supplied call back
				if ( (strcmp(ext, szExtension) == 0) || (strcmp(szExtension, "*") ==0) ) 
				{
					// build the full file name 
					_getcwd(szFullFileName, MAX_PATH);
					strcat(szFullFileName, "\\");
					strcat(szFullFileName, fileFindData.cFileName);


					// call back with the string and parameter
					cbFunc(szFullFileName, pParam);
				}
			}
		}
		
		if (!FindNextFile(dirHandle, &fileFindData)) break;
	}
	
	
	FindClose(dirHandle);

	_chdir(szRestoreDir);
}// END ScanForFileTypes


void DeleteFolderContentsCb(char* fn, void* pParam)
{
	if(IsDirectory(fn))	   
	{
		DeleteFolderContents(fn);
	}
	else
	{
		remove(fn);
	}
}// END DeleteFolderContentsCb



// deletes the passed folder and everything in it, be VERY careful with this
void DeleteFolderContents(const char *szFolder)
{
	if(IsDirectory(szFolder))
	{
		ScanForFileTypes(szFolder, "*", DeleteFolderContentsCb, NULL);
	}
}// END DeleteFolderContents


}// namespace FileHelpers