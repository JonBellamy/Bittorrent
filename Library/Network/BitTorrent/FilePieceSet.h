// Jon Bellamy 21/02/2009


#ifndef FILE_PIECE_SET_H
#define FILE_PIECE_SET_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <string>
#include <vector>

#include "File/file.h"
#include "Network/BitTorrent/BitTorrentMetaFile.h"
#include "Network/BitTorrent/BEncoding/BencodedDictionary.h"


class cBitTorrent;


class cFilePieceSet
{
public:
    cFilePieceSet(const cBitTorrentMetaFile& metaFile, const char* rootDir);
	~cFilePieceSet();

private:
	cFilePieceSet(const cFilePieceSet&);
	const cFilePieceSet& operator= (const cFilePieceSet& rhs);


public:

	void CloseAll();

	bool OpenAllFiles(bool createFiles);

	void Flush();

	typedef enum {READ, WRITE} PieceOperation;
	bool ReadWritePiece(PieceOperation opType, s64 pieceIndex, s64 begin, s64 blockSize, u8* bufOut);

	// check the pieces hash
	bool IsLocalPieceValid(s64 pieceIndex);

	// debug only, slow
	bool ValidateAll();

	s64 TotalFileSize() const;
	u32 PieceSize() const;
	u32 FinalPieceSize() const;
	u32 NumberOfPieces() const;

	u32 NumberOfFiles() const;
	std::string FileName(u32 fileIndex) const;
	s64 FileSize(u32 fileIndex) const;
	u32 FileStartPiece(u32 fileIndex) const;
	u32 FileEndPiece(u32 fileIndex) const;
	u32 NumPiecesFileIsSpreadOver(u32 fileIndex) const;

	const std::string& RootFolder() const { return mRootFolder; }
	
	
	typedef enum
	{
		FCR_CLOSED = 0,
		FCR_WORKING,
		FCR_SUCCESS,
		FCR_FAILED,
	}FileCreationResult;
	FileCreationResult FilesCreationState() const { return mFilesCreated; }

	void CancelFileCreation();

private:

	void CalcTotalFileSize();

	typedef struct
	{
		cFileStream64Bit* mFile;
		s64 mFileSize;
		bool mNeedToSize;
	}FileListElement;

	static void* CreateFiles(void* pParam);

	// returns the index into mFileList for the past byte
	u32 FileListIndexFromStreamOffset(s64 byteOffset) const;

	// returns the offset into the file where the passed stream offset lands
	s64 FileOffsetFromStreamOffest(s64 byteOffset) const;

	u32 PieceOffsetFromStreamOffset(s64 byteOffset) const;

	// returns the stream offset where the passed file index starts
	s64 FileStartStreamOffset(u32 fileIndex) const;
	s64 FileEndStreamOffset(u32 fileIndex) const;

	// does the past sub piece cross a file boundary
	bool DoesPieceSpanFileBoundary(s64 pieceIndex, s64 begin, s64 blockSize) const;

	// Gets the precomputed hash for the passed piece. Used to check our downloaded piece is valid. pHashOut must have storage for INFO_HASH_LENGTH bytes
	void PieceHash(s64 pieceIndex, u8* pHashOut) const;


	const cBitTorrentMetaFile& mMetaFile;

	// ordered file list, piece 0 will be the first part of the first file in this list
	std::vector<FileListElement> mFileList;

	// where are we going to put these files
	std::string mRootFolder;

	u32 mPieceLength;

	// string consisting of the concatenation of all 20-byte SHA1 hash values, one per piece (byte string, i.e. not urlencoded) 
	std::string mPieceHashes;

	s64 mTotalFileSize;

	// Set when the files are created / opened
	FileCreationResult mFilesCreated;

	//pthread_t mFileCreationThread;
};



inline s64 cFilePieceSet::TotalFileSize() const
{
	return mTotalFileSize;
}// END TotalFileSize







#endif // FILE_PIECE_SET_H
