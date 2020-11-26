// Jon Bellamy 20/02/2009


#ifndef BITTORRENT_META_H
#define BITTORRENT_META_H

#if USE_PCH
#include "stdafx.h"
#endif


#include "Network/BitTorrent/BitTorrentValues.h"
#include "BEncoding/BencodedDictionary.h"



class cBitTorrentMetaFile
{
public:
    cBitTorrentMetaFile();
	~cBitTorrentMetaFile();

	bool Parse(const char* fn);

private:
	cBitTorrentMetaFile(const cBitTorrentMetaFile&);
	const cBitTorrentMetaFile& operator= (const cBitTorrentMetaFile& rhs);


public:
	
	bool IsSingleFile() const;
	u64 TotalSizeOfContentInBytes() const;

	const cBencodedDictionary& RootDictionary() const;
	const cBencodedDictionary* InfoDictionary() const;
	
	std::string Name() const;
	std::string Comment() const;
	u32 CreationDate() const;

private:


	std::string mMetaFileName;

	cBencodedDictionary mMetaFile;
};





//////////////////////////////////////////////////////////////////////////
// inlines


inline const cBencodedDictionary& cBitTorrentMetaFile::RootDictionary() const
{
	return mMetaFile;
}// END RootDictionary



inline const cBencodedDictionary* cBitTorrentMetaFile::InfoDictionary() const
{
	return static_cast<const cBencodedDictionary*>(mMetaFile.GetValue("info"));
}// END InfoDictionary





#endif // BITTORRENT_META_H
