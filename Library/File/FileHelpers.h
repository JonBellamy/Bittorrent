// Jon Bellamy
// 28/04/2004
// Helper Function Lib 


#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#if USE_PCH
#include "stdafx.h"
#endif


#include <stdio.h>


namespace FileHelpers
{

//////////////////////////////////////////////////////////////////////////
// File Helpers

// tests if a file exists, does not alter or open the file 
bool FileExists( const char *szFileName );

// the passed file pointer must be open, it will not be closed and the file pointer will be returned to the begining 
s32 GetFileSize( FILE *pFile );
s32 GetFileSize(const char* fn );

bool IsDirectory( const char *fileName );

bool MkDir( const char *fileName );

// ensures that all the folders in the passed file name exist and creates any that don't
bool MakeAllDirs( const char *fileName );
// call back function typedef for the following file find function
// The type in front matches the function return type, and the types in back match the argument list.
typedef void (*CBFileFind) (char *, void *);

// recursive function that scans the passed folder and all sub folders for the given file extension
void ScanForFileTypes( const char *szSearchFolder, char *szExtension, CBFileFind cbFunc, void *pParam);

void DeleteFolderContents(const char *szFolder);



}// END namespace FileHelpers


#endif // FILE_HELPERS_H