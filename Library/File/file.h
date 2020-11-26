// Jon Bellamy 23-09-2001
// File wrapper


#ifndef FILE_H
#define FILE_H

#if USE_PCH
#include "stdafx.h"
#endif


#include <assert.h>
#include <stdio.h>
#include <string>


class cFile
{
public:

	typedef enum 
	{
		WRITE=0,			// destroys the contents pf the file if it exists
		READ,
		APPEND,				// reading and writing 
		FM_ERR
	}FileMode;
	
	enum
	{
		FILE_POS_CURRENT = -1
	};

	cFile(FileMode mode, const char *szFileName, bool cacheFile=false);
	cFile();
	virtual ~cFile();

	virtual bool Open(FileMode mode, const char *szFileName, bool cacheFile=false);
	virtual void Close();

	bool Cache();

	u32 Size() const;

	bool Read(s64 filePos, u32 size, u8* outBuffer);
	
	// filePos==FILE_POS_CURRENT writes to end of file, remember you cannot insert bytes into the middle of a file
	bool Write(s64 filePos, u32 size, const u8* bytes);

	void Flush() const;

	const char* Name() const { return mFn; }

	u8* CachedFileData() { return mpCachedFileData; }

	bool IsOpen() const { return pFile != NULL; }

private:

	FILE *pFile;
	FileMode mFileMode;
	char mFn[256];
	
	u8* mpCachedFileData;
};


#define USE_FILE64 1

#if USE_FILE64

// instead of windows.h gor for winsock to spare me the compiler issues, you get windows.h anyway
#include <WinSock2.h>

// the CRT is very flaky when dealing with large files (4GB+), the crashes it produces are spectacular. Use this windows specific variant to write to big files
class cFileStream64Bit
{
public:
	cFileStream64Bit();
	virtual ~cFileStream64Bit();

	bool Open(const std::string& fileName);
	void Close();

	bool Read(s64 filePos, u32 size, u8* outBuffer);

	bool Write(s64 filePos, u32 size, const u8* bytes);

	s64 Size() const;

	void Flush() const;

	const std::string& Name() const { return mFn; }

	static void ReplaceInvalidCharsInFilename(std::string& fn);

private:

	HANDLE mFile;
	std::string mFn;
};

#endif //  USE_FILE64


#endif // FILE_H