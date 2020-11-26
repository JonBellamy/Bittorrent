// Jon Bellamy 20/02/2009


#include "BitTorrentMetaFile.h"


#include <stdio.h>
#include <assert.h>

#include "File/file.h"
#include "Network/BitTorrent/BEncoding/BencodedString.h"
#include "Network/BitTorrent/BEncoding/BencodedInt.h"
#include "Network/BitTorrent/BEncoding/BencodedList.h"



cBitTorrentMetaFile::cBitTorrentMetaFile()
{
}// END cBitTorrentMetaFile



cBitTorrentMetaFile::~cBitTorrentMetaFile()
{
}// END ~cBitTorrentMetaFile



bool cBitTorrentMetaFile::Parse(const char* fn)
{
	mMetaFileName = fn;

	cFile tFile;
	if(tFile.Open(cFile::READ, mMetaFileName.c_str(), true) == false)
	{
		return false;
	}

	if(!mMetaFile.Parse(reinterpret_cast<const char *>(tFile.CachedFileData()), tFile.Size()))
	{
		return false;
	}
	return true;
}// END Parse



bool cBitTorrentMetaFile::IsSingleFile() const
{
	const cBencodedDictionary* infoDict = InfoDictionary();
	assert(infoDict && infoDict->Type() == cBencodedType::BEN_DICTIONARY);
	if(infoDict)
	{
		if(infoDict->GetValue("files") != NULL)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	assert(0);
	return false;
}// END IsSingleFile



u64 cBitTorrentMetaFile::TotalSizeOfContentInBytes() const
{
	const cBencodedDictionary* infoDict = InfoDictionary();
	assert(infoDict);
	if(infoDict)
	{
		if(IsSingleFile())
		{
			const cBencodedInt* benInt = static_cast<const cBencodedInt*>(infoDict->GetValue("length")); 
			assert(benInt && benInt->Type() == cBencodedType::BEN_INT);
			if(benInt)
			{
				return benInt->Get();
			}
			return 0;
		}
		else
		{
			const cBencodedList* benList = static_cast<const cBencodedList*>(infoDict->GetValue("files")); 
			assert(benList);
			if(benList)
			{
				s64 size=0;
				for(u32 i=0; i < benList->NumElements(); i++)
				{
					const cBencodedDictionary* fileDict = static_cast<const cBencodedDictionary*>(benList->GetElement(i));
					assert(fileDict && fileDict->Type() == cBencodedType::BEN_DICTIONARY);

					const cBencodedInt* benInt = static_cast<const cBencodedInt*>(fileDict->GetValue("length")); 
					assert(benInt && benInt->Type() == cBencodedType::BEN_INT);
					if(benInt)
					{
						size += benInt->Get();
					}
					else
					{
						assert(0);
						return 0;
					}
				}
				return size;
			}
			return 0;
		}
	}

	assert(0);
	return 0;
}// END TotalSizeOfContentInBytes



std::string cBitTorrentMetaFile::Name() const 
{ 
	const cBencodedString* bStr = reinterpret_cast<const cBencodedString*> (InfoDictionary()->GetValue("name"));
	if(!bStr || bStr->Type() != cBencodedType::BEN_STRING)
	{
		return "UNNAMED TORRENT";
	}
	return bStr->Get();
}// END Name



std::string cBitTorrentMetaFile::Comment() const
{
	const cBencodedString* bStr = reinterpret_cast<const cBencodedString*> (RootDictionary().GetValue("comment"));
	if(!bStr || bStr->Type() != cBencodedType::BEN_STRING)
	{
		return " ";
	}
	return bStr->Get();
}// END Comment



// (optional) the creation time of the torrent, in standard UNIX epoch format (integer, seconds since 1-Jan-1970 00:00:00 UTC) 
u32 cBitTorrentMetaFile::CreationDate() const
{
	const cBencodedInt* bInt = reinterpret_cast<const cBencodedInt*> (RootDictionary().GetValue("creation date"));
	if(!bInt || bInt->Type() != cBencodedType::BEN_INT)
	{
		return 0;
	}
	return (u32) bInt->Get();
}// END CreationDate