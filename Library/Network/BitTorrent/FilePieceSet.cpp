// Jon Bellamy 21/02/2009


#include "FilePieceSet.h"

#include <assert.h>
#include <direct.h>
#include <math.h>

#include "Network/BitTorrent/BitTorrent.h"
#include "Network/BitTorrent/BEncoding/BencodedString.h"
#include "Network/BitTorrent/BEncoding/BencodedInt.h"
#include "Network/BitTorrent/BEncoding/BencodedList.h"
#include "File/FileHelpers.h"
#include "openssl/sha.h"


cFilePieceSet::cFilePieceSet(const cBitTorrentMetaFile& metaFile, const char* rootDir)
: mMetaFile(metaFile)
, mRootFolder(rootDir)
, mPieceLength(0)
, mTotalFileSize(0)
, mFilesCreated(FCR_CLOSED)
{
}// END cFilePieceSet



cFilePieceSet::~cFilePieceSet()
{
	CloseAll();
}// END ~cFilePieceSet



void cFilePieceSet::CloseAll()
{
	for(u32 i=0; i < mFileList.size(); i++)
	{
		FileListElement f = mFileList[i];
		f.mFile->Close();
		delete f.mFile;
		f.mFile = NULL;
	}
	mFileList.clear();
}// END CloseAll



bool cFilePieceSet::OpenAllFiles(bool createFiles)
{
	mFilesCreated = FCR_WORKING;

	char cwd[512];
	_getcwd(cwd, sizeof(cwd));

	if(_chdir(mRootFolder.c_str()) != 0)
	{
		return false;
	}

	if(mMetaFile.InfoDictionary() == NULL)
	{
		assert(0);
		goto fail;
	}

	/////////////////////////////////////////////////////////////////////
	// common file stuff ...

	// piece length
	const cBencodedInt* pBenPieceLengthInt = static_cast<const cBencodedInt*> (mMetaFile.InfoDictionary()->GetValue("piece length"));
	assert(pBenPieceLengthInt && pBenPieceLengthInt->Type() == cBencodedType::BEN_INT);
	if(pBenPieceLengthInt==NULL || pBenPieceLengthInt->Type() != cBencodedType::BEN_INT)
	{
		goto fail;
	}
	mPieceLength = static_cast<u32> (pBenPieceLengthInt->Get());


	const cBencodedString* pBenStringPiecesHash = static_cast<const cBencodedString*> (mMetaFile.InfoDictionary()->GetValue("pieces"));
	assert(pBenStringPiecesHash && pBenStringPiecesHash->Type() == cBencodedType::BEN_STRING);
	if(pBenStringPiecesHash==NULL || pBenStringPiecesHash->Type() != cBencodedType::BEN_STRING)
	{
		goto fail;
	}
	mPieceHashes = pBenStringPiecesHash->Get();
	if(mPieceHashes.size() % INFO_HASH_LENGTH != 0)
	{
		assert(0);
		goto fail;
	}


	bool filesToCreate = false;

	/////////////////////////////////////////////////////////////////////
	// single and multi file specific stuff ...

	if(mMetaFile.IsSingleFile())
	{
		// single file 

		// filename
		std::string fn;
		const cBencodedString* pBenNameString = static_cast<const cBencodedString*> (mMetaFile.InfoDictionary()->GetValue("name"));
		assert(pBenNameString && pBenNameString->Type() == cBencodedType::BEN_STRING);
		if(pBenNameString==NULL || pBenNameString->Type() != cBencodedType::BEN_STRING)
		{
			goto fail;
		}
		fn = pBenNameString->Get();

		// file length
		s64 fileLength;
		const cBencodedInt* pBenIntLength = static_cast<const cBencodedInt*> (mMetaFile.InfoDictionary()->GetValue("length"));
		assert(pBenIntLength && pBenIntLength->Type() == cBencodedType::BEN_INT);
		if(pBenIntLength==NULL || pBenIntLength->Type() != cBencodedType::BEN_INT)
		{
			goto fail;
		}
		fileLength = pBenIntLength->Get();
		assert(fileLength > 0);
		if(fileLength <= 0)
		{
			goto fail;
		}


		// there is also the 'md5sum' key here if we need it, although it is optional

		bool fileExists = FileHelpers::FileExists(fn.c_str());

		cFileStream64Bit::ReplaceInvalidCharsInFilename(fn);

		// create the file
		cFileStream64Bit* pFile = new cFileStream64Bit;
		bool ret = pFile->Open(fn);
		if(ret == false)
		{
			goto fail;
		}


		FileListElement flElement;
		flElement.mFile = pFile;
		flElement.mFileSize = fileLength;
		flElement.mNeedToSize = false;

		if(createFiles && !fileExists)
		{
			filesToCreate = true;
			flElement.mNeedToSize = true;
		}

		mFileList.push_back(flElement);
	}
	else
	{
		// multi-file

		// the filename of the directory in which to store all the files
		std::string strDirectory;
		const cBencodedString* pBenNameString = static_cast<const cBencodedString*> (mMetaFile.InfoDictionary()->GetValue("name"));
		assert(pBenNameString && pBenNameString->Type() == cBencodedType::BEN_STRING);
		if(pBenNameString==NULL || pBenNameString->Type() != cBencodedType::BEN_STRING)
		{
			goto fail;
		}
		strDirectory = pBenNameString->Get();


		// files is a list of dictionaries, one for each file. Each dictionary in this list contains the following keys:
		// length: length of the file in bytes (integer)
		// md5sum: (optional) a 32-character hexadecimal string corresponding to the MD5 sum of the file. This is not used by BitTorrent at all, but it is included by some programs for greater compatibility.
		// path:   a list containing one or more string elements that together represent the path and filename. 
		//           Each element in the list corresponds to either a directory name or (in the case of the final element)
		//           the filename. For example, a the file "dir1/dir2/file.ext" would consist of three string elements: 
		//           "dir1", "dir2", and "file.ext". 

		const cBencodedList* pBencodedFilesList = static_cast<const cBencodedList*> (mMetaFile.InfoDictionary()->GetValue("files"));
		assert(pBencodedFilesList && pBencodedFilesList->Type() == cBencodedType::BEN_LIST);
		if(pBencodedFilesList==NULL || pBencodedFilesList->Type() != cBencodedType::BEN_LIST)
		{
			goto fail;
		}

		for(u32 i=0; i < pBencodedFilesList->NumElements(); i++)
		{
			const cBencodedDictionary* pBencodedFileDict = static_cast<const cBencodedDictionary*> (pBencodedFilesList->GetElement(i));
			assert(pBencodedFileDict && pBencodedFileDict->Type() == cBencodedType::BEN_DICTIONARY);
			if(pBencodedFileDict==NULL || pBencodedFileDict->Type() != cBencodedType::BEN_DICTIONARY)
			{
				goto fail;
			}


			// file length
			s64 fileLength;
			const cBencodedInt* pBenIntLength = static_cast<const cBencodedInt*> (pBencodedFileDict->GetValue("length"));
			assert(pBenIntLength && pBenIntLength->Type() == cBencodedType::BEN_INT);
			if(pBenIntLength==NULL || pBenIntLength->Type() != cBencodedType::BEN_INT)
			{
				goto fail;
			}
			fileLength = pBenIntLength->Get();
			//assert(fileLength > 0);
			if(fileLength < 0)
			{
				goto fail;
			}


			const cBencodedList* pBencodedFilePathList = static_cast<const cBencodedList*> (pBencodedFileDict->GetValue("path"));
			assert(pBencodedFilePathList && pBencodedFilePathList->Type() == cBencodedType::BEN_LIST);
			if(pBencodedFilePathList==NULL || pBencodedFilePathList->Type() != cBencodedType::BEN_LIST)
			{
				goto fail;
			}
			// build the path list
			std::string strFileAndPath = strDirectory;			
			for(u32 j=0; j < pBencodedFilePathList->NumElements(); j++)
			{
				strFileAndPath += "\\";

				const cBencodedString* pBenPathElementString = static_cast<const cBencodedString*> (pBencodedFilePathList->GetElement(j));
				assert(pBenPathElementString && pBenPathElementString->Type() == cBencodedType::BEN_STRING);
				if(pBenPathElementString==NULL || pBenPathElementString->Type() != cBencodedType::BEN_STRING)
				{
					goto fail;
				}

				strFileAndPath += pBenPathElementString->Get();
			}

			if(!FileHelpers::MakeAllDirs(strFileAndPath.c_str()))
			{
				assert(0);
				goto fail;
			}


			// there is also the 'md5sum' key here if we need it, although it is optional

			cFileStream64Bit::ReplaceInvalidCharsInFilename(strFileAndPath);

			bool fileExists = FileHelpers::FileExists(strFileAndPath.c_str());

			// create the file
			cFileStream64Bit* pFile = new cFileStream64Bit;
			bool ret = pFile->Open(strFileAndPath);
			if(ret == false)
			{
				goto fail;
			}


			// add to the list
			FileListElement flElement;
			flElement.mFile = pFile;
			flElement.mFileSize = fileLength;
			flElement.mNeedToSize = false;

			if( createFiles && 
				(!fileExists || flElement.mFile->Size() != flElement.mFileSize) )
			{
				filesToCreate = true;
				flElement.mNeedToSize = true;
			}

			mFileList.push_back(flElement);
		}

	}


	_chdir(cwd);

	CalcTotalFileSize();

	if(TotalFileSize() != mMetaFile.TotalSizeOfContentInBytes())
	{
		assert(0);
		return false;
	}

	if(filesToCreate)
	{
		// If we get here, the files are all open but they are zero size, we need to write a byte to create the files at the correct size
		// this can take some time, so we thread it.	
		//int rc = pthread_create(&mFileCreationThread, NULL, CreateFiles, this);
		//ASSERT(rc == 0);

		CreateFiles(NULL);
	}
	else
	{
		mFilesCreated = FCR_SUCCESS;
	}

	return true;

fail:
	_chdir(cwd);

	mFilesCreated = FCR_FAILED;

	return false;
}// END OpenAllFiles



void* cFilePieceSet::CreateFiles(void* pParam)
{
	cFilePieceSet* pThis = reinterpret_cast<cFilePieceSet*> (pParam);
	for(u32 i=0; i < pThis->mFileList.size(); i++)
	{
		FileListElement f = pThis->mFileList[i];
		if(f.mFile && f.mNeedToSize)
		{
			u8 tmpByte=0;
			f.mFile->Write(f.mFileSize-1, 1, &tmpByte);
			f.mFile->Flush();
			f.mNeedToSize = false;
		}
	}

	pThis->mFilesCreated = FCR_SUCCESS;

	return NULL;
}// END CreateFiles



void cFilePieceSet::CancelFileCreation()
{
	//pthread_cancel(mFileCreationThread);
}// END CancelFileCreation



void cFilePieceSet::Flush()
{
	for(u32 i=0; i < mFileList.size(); i++)
	{
		FileListElement f = mFileList[i];
		if(f.mFile)
		{
			f.mFile->Flush();
		}
	}
}// END Flush


bool cFilePieceSet::ReadWritePiece(PieceOperation opType, s64 pieceIndex, s64 begin, s64 blockSize, u8* buf)
{
	if(pieceIndex > NumberOfPieces())
	{
		assert(0);
		return false;
	}

	bool bRet;
	s64 bytesWritten=0;
	s64 bytesCanWriteToThisFile;
	s64 startByte = begin + (pieceIndex * PieceSize());

	// write to one file per iteration
	while(bytesWritten != blockSize)
	{
		s64 streamOffset = startByte + bytesWritten;
		u32 fileIndex = FileListIndexFromStreamOffset(streamOffset);
		cFileStream64Bit* pFile = mFileList[fileIndex].mFile;

		bytesCanWriteToThisFile = FileSize(fileIndex) - FileOffsetFromStreamOffest(streamOffset);

		if(bytesCanWriteToThisFile >= (blockSize - bytesWritten))
		{
			// can read/write all data
			bytesCanWriteToThisFile = (blockSize - bytesWritten);
		}

		switch(opType)
		{
		case READ:
			bRet = pFile->Read(FileOffsetFromStreamOffest(streamOffset), static_cast<u32>(bytesCanWriteToThisFile), buf + bytesWritten);
			break;

		case WRITE:
			bRet = pFile->Write(FileOffsetFromStreamOffest(streamOffset), static_cast<u32>(bytesCanWriteToThisFile), buf + bytesWritten);
			break;

		default:
			assert(0);
		}


		bytesWritten += bytesCanWriteToThisFile;


		if(bytesWritten > blockSize || !bRet)
		{
			assert(0);
			return false;
		}
	}

	return true;
}// END ReadWritePiece



// this could block for a while
bool cFilePieceSet::IsLocalPieceValid(s64 pieceIndex)
{
	u32 pieceSize;

	
	u8 expectedPieceHash[INFO_HASH_LENGTH];
	u8 localPieceHash[INFO_HASH_LENGTH];

	PieceHash(pieceIndex, expectedPieceHash);

	if(pieceIndex == NumberOfPieces()-1)
	{
		pieceSize = FinalPieceSize();
	}
	else
	{
		pieceSize = PieceSize();
	}

	u8* buf = new u8[pieceSize];

	ReadWritePiece(READ, pieceIndex, 0, pieceSize, buf);
	SHA1(buf, pieceSize, localPieceHash);

	delete[] buf;


	return (memcmp(localPieceHash, expectedPieceHash, INFO_HASH_LENGTH) == 0);
}// END IsLocalPieceValid



// debug only, slow
bool cFilePieceSet::ValidateAll()
{
	Flush();
	for(u32 i=0; i < NumberOfPieces(); i++)
	{
		if(!IsLocalPieceValid(i))
		{
			Printf("**** piece %d failed hash check!\n", i);

#if 0
			u8* buf = new u8[PieceSize()];
			ReadWritePiece(READ, i, 0, PieceSize(), buf);

			char str[256];
			sprintf(str, "c:\\torrenttest\\%dDL PIECE BAD", i);
			cFile f(cFile::WRITE, str);
			f.Write(0, PieceSize(), buf);
			f.Close();
			delete[] buf;
#endif


			return false;
		}
	}
	return true;
}// END ValidateAll



// returns the index into mFileList for the past byte
u32 cFilePieceSet::FileListIndexFromStreamOffset(s64 byteOffset) const
{
	s64 streamPos=0;
	for(u32 i=0; i < mFileList.size(); i++)
	{
		const FileListElement& fe = mFileList[i];
			
		if(byteOffset < streamPos + fe.mFileSize)
		{
			return i;
		}

		streamPos += fe.mFileSize;
	}

	assert(0);
	return 0;
}// END FileListIndexFromStreamOffset



// returns the offset into the file where the passed stream offset lands
s64 cFilePieceSet::FileOffsetFromStreamOffest(s64 byteOffset) const
{
	if(byteOffset > TotalFileSize())
	{
		assert(0);
		return false;
	}

	// find which file it lands in
	u32 fileIndex = FileListIndexFromStreamOffset(byteOffset);

	// offset - fileStartpos
	return byteOffset - FileStartStreamOffset(fileIndex);
}// END FileOffsetFromStreamOffest



u32 cFilePieceSet::PieceOffsetFromStreamOffset(s64 byteOffset) const
{
	if(byteOffset > TotalFileSize())
	{
		assert(0);
		return false;
	}

	return u32(byteOffset / PieceSize());
}// END PieceOffsetFromStreamOffset



// returns the stream offset where the passed file index starts
s64 cFilePieceSet::FileStartStreamOffset(u32 fileIndex) const
{
	if(fileIndex >= mFileList.size())
	{
		assert(0);
		return false;
	}

	s64 streamPos=0;
	for(u32 i=0; i < mFileList.size(); i++)
	{
		const FileListElement& fe = mFileList[i];

		if(i == fileIndex)
		{
			return streamPos;
		}
		
		streamPos += fe.mFileSize;
	}

	assert(0);
	return 0;
}// END FileStartStreamOffset



s64 cFilePieceSet::FileEndStreamOffset(u32 fileIndex) const
{
	if(fileIndex >= mFileList.size())
	{
		assert(0);
		return false;
	}

	s64 streamPos=0;
	for(u32 i=0; i < mFileList.size(); i++)
	{
		const FileListElement& fe = mFileList[i];

		if(i == fileIndex)
		{
			return streamPos + fe.mFileSize;
		}
	
		streamPos += fe.mFileSize;
	}

	assert(0);
	return 0;
}// END FileEndStreamOffset




// does the past sub piece cross a file boundary
bool cFilePieceSet::DoesPieceSpanFileBoundary(s64 pieceIndex, s64 begin, s64 blockSize) const
{
	s64 startByte = begin + (pieceIndex * PieceSize());

	s64 streamPos=0;
	for(u32 i=0; i < mFileList.size(); i++)
	{
		const FileListElement& fe = mFileList[i];

		if(streamPos + fe.mFileSize >= startByte)
		{
			// we start in this piece
			if(startByte + blockSize >= streamPos + fe.mFileSize)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		streamPos += fe.mFileSize;		
	}

	return false;
}// END DoesPieceSpanFileBoundary



// Gets the precomputed hash for the passed piece. Used to check our downloaded piece is valid. pHashOut must have storage for INFO_HASH_LENGTH bytes
void cFilePieceSet::PieceHash(s64 pieceIndex, u8* pHashOut) const
{
	memcpy(pHashOut, mPieceHashes.c_str() + (pieceIndex * INFO_HASH_LENGTH), INFO_HASH_LENGTH);
}// END PieceHash



void cFilePieceSet::CalcTotalFileSize()
{
	s64 size=0;
	for(u32 i=0; i < mFileList.size(); i++)
	{
		const FileListElement& fe = mFileList[i];
		size += fe.mFileSize;
	}
	mTotalFileSize = size;
}// END CalcTotalFileSize



u32 cFilePieceSet::PieceSize() const
{
	return mPieceLength;
}// END PieceSize



u32 cFilePieceSet::FinalPieceSize() const
{
	s64 size = TotalFileSize() % PieceSize();
	// Check if final piece just happens to be the same size as all the others
	if(size == 0)
	{
		return PieceSize();
	}
	return static_cast<u32> (size);
}// END FinalPieceSize



u32 cFilePieceSet::NumberOfPieces() const
{
	double d = ceil(double(TotalFileSize()) / double(PieceSize()));
	return static_cast<u32> (d);
}// END NumberOfPieces



u32 cFilePieceSet::NumberOfFiles() const
{
	return  mFileList.size();
}// END NumberOfFiles



std::string cFilePieceSet::FileName(u32 fileIndex) const
{
	if(fileIndex > mFileList.size())
	{
		assert(0);
		return "";
	}
	return mFileList[fileIndex].mFile->Name();
}// END FileName



s64 cFilePieceSet::FileSize(u32 fileIndex) const
{
	if(fileIndex > mFileList.size())
	{
		assert(0);
		return 0;
	}
	return mFileList[fileIndex].mFileSize;
}// END FileSize



u32 cFilePieceSet::FileStartPiece(u32 fileIndex) const
{
	if(fileIndex > mFileList.size())
	{
		assert(0);
		return 0;
	}
	return PieceOffsetFromStreamOffset(FileStartStreamOffset(fileIndex));
}// END FileStartPiece



u32 cFilePieceSet::FileEndPiece(u32 fileIndex) const
{
	if(fileIndex > mFileList.size())
	{
		assert(0);
		return 0;
	}
	return PieceOffsetFromStreamOffset(FileEndStreamOffset(fileIndex));
}// END FileEndPiece



u32 cFilePieceSet::NumPiecesFileIsSpreadOver(u32 fileIndex) const
{
	return 1 + (FileEndPiece(fileIndex) - FileStartPiece(fileIndex));
}// END NumPiecesFileIsSpreadOver