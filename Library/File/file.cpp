// Jon Bellamy 23-09-2001
// Logs events into a text file, used in this case to generate the lua files

#if USE_PCH
#include "stdafx.h"
#endif


#include "file.h"

#include <string.h>
#include <stdarg.h>

#include "File/FileHelpers.h"


cFile::cFile()
: mFileMode(FM_ERR)
, pFile(NULL)
, mpCachedFileData(NULL)
{
}// END cFile



cFile::cFile(FileMode mode, const char *szFileName, bool cacheFile)
: mFileMode(mode)
, pFile(NULL)
, mpCachedFileData(NULL)
{
	Open(mode, szFileName, cacheFile);
}// END cFile



cFile::~cFile()
{
	Close();

	if(mpCachedFileData)
	{
		delete[] mpCachedFileData;
	}
}// END ~cFile



bool cFile::Open(FileMode mode, const char *szFileName, bool cacheFile)
{
	strcpy(mFn, szFileName);

	mFileMode = mode;

	switch(mFileMode)
	{
	case WRITE:
		{
			// delete the old file
			//remove(szFileName);
			pFile = fopen(szFileName, "wb");
			break;
		}
	case READ:
		{
			pFile = fopen(szFileName, "rb");
			break;
		}

	case APPEND:
		{
			pFile = fopen(szFileName, "a+b");
			break;
		}
	default:
		assert(0);
	}

	if (pFile == NULL)
	{		
		s32 err;
		_get_errno(&err);
		Printf("Failed to create/open file, the file is probably read only. errno = %d\n", err);
		return false;
	}
	else
	{
		if(cacheFile)
		{
			Cache();
		}
	}

	return true;
}// END Open



void cFile::Close()
{	
	if (pFile) 
	{
		Flush();
		fclose(pFile);
		pFile=NULL;
		mFileMode = FM_ERR;
	}
}// END Close



bool cFile::Cache()
{
	if(mpCachedFileData)
	{
		delete[] mpCachedFileData;
		mpCachedFileData = NULL;
	}

	u32 size = Size();
	mpCachedFileData = new u8[size];
	if(!mpCachedFileData)
	{
		return false;
	}
	
	Read(0, size, mpCachedFileData);
	//Close();


	return true;
}// END Cache



// TODO : this should cache the value
u32 cFile::Size() const
{
	if(!pFile)
	{
		return 0;
	}
	Flush();
	return FileHelpers::GetFileSize(pFile);
}// END Size



bool cFile::Read(s64 filePos, u32 size, u8* outBuffer)
{
	if(!pFile ||
		(filePos + size > Size()))
	{
		return false;
	}

	s32 ret;
	ret = _fseeki64(pFile, filePos, SEEK_SET);
	if(ret != 0)
	{
		return false;
	}

	fread(outBuffer, size, 1, pFile);
	if(ret != 0)
	{
		return false;
	}

	return true;
}// END Read



// filePos==-1 writes to end of file, remember you cannot insert bytes into the middle of a file
bool cFile::Write(s64 filePos, u32 size, const u8* bytes)
{
	if(!pFile ||
		size == 0)
	{
		assert(0);
		return false;
	}

	s32 ret;

	if(filePos == -1)
	{
		ret = _fseeki64(pFile, 0, SEEK_END);
	}
	else
	{
		ret = _fseeki64(pFile, filePos, SEEK_SET);
	}
	if(ret != 0)
	{
		return false;
	}

	ret = static_cast<s32>(fwrite(bytes, size, 1, pFile));
	if(ret != 1)
	{
		return false;
	}

	return true;
}// END Write



void cFile::Flush() const
{
	if(pFile)
	{
		fflush(pFile);
	}
}// END Flush




#if USE_FILE64



cFileStream64Bit::cFileStream64Bit()
: mFile(INVALID_HANDLE_VALUE)
{
}



cFileStream64Bit::~cFileStream64Bit()
{
	Close();
}



bool cFileStream64Bit::Open(const std::string& fileName)
{
	mFn = fileName;

	WCHAR outf[2048];
	memset(outf, 0, sizeof(outf));
	int r;
	r = MultiByteToWideChar(CP_UTF8, 0, mFn.c_str(), -1, outf, 2048);
	if(r == 0)
	{
		return false;
	}

	mFile = CreateFileW(outf, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(mFile == INVALID_HANDLE_VALUE)
	{
		u32 err = GetLastError();
		LPTSTR lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			0, // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
			);

		assert(0);

		return false;
	}
	return true;
}



void cFileStream64Bit::Close()
{
	if(mFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(mFile);
		mFile = INVALID_HANDLE_VALUE;
	}
}



bool cFileStream64Bit::Read(s64 filePos, u32 size, u8* outBuffer)
{
	if(mFile == INVALID_HANDLE_VALUE || filePos < 0)
	{
		return false;
	}
	DWORD numBytesRead;
	LARGE_INTEGER li;
	li.QuadPart = filePos;
	SetFilePointerEx(mFile, li, NULL, FILE_BEGIN);
	ReadFile(mFile, outBuffer, size, &numBytesRead, NULL);

	if(numBytesRead != size)
	{
		return false;
	}

	return true;
}


bool cFileStream64Bit::Write(s64 filePos, u32 size, const u8* bytes)
{
	if(mFile == INVALID_HANDLE_VALUE || filePos < 0)
	{
		return false;
	}
	DWORD numBytesWritten;
	LARGE_INTEGER li;
	li.QuadPart = filePos;
	SetFilePointerEx(mFile, li, NULL, FILE_BEGIN);
	WriteFile(mFile, bytes, size, &numBytesWritten, NULL);

	if(numBytesWritten != size)
	{
		return false;
	}

	return true;
}



s64  cFileStream64Bit::Size() const
{
	LARGE_INTEGER s;
	GetFileSizeEx(mFile, &s);
	return s.QuadPart;
}// END Size



void cFileStream64Bit::Flush() const
{
	if(mFile != INVALID_HANDLE_VALUE)
	{
		FlushFileBuffers(mFile);
	}
}



// Replaces invalid chars in a filename with _
void cFileStream64Bit::ReplaceInvalidCharsInFilename(std::string& fn)
{
	char invalidChars[] = "<>*?\"";
	for(u32 i=0; i < fn.size(); i++)
	{
		for(u32 j=0; j < sizeof(invalidChars)-1; j++)
		{
			if(fn[i] == invalidChars[j])
			{
				fn[i] = '_';
				break;
			}
		}		
	}
}// END ReplaceInvalidCharsInFilename





#endif // USE_FILE64

