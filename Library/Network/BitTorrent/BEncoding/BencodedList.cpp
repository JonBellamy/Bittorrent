// Jon Bellamy

// Lists are encoded as follows: l<bencoded values>e
// The initial l and trailing e are beginning and ending delimiters. Lists may contain any bencoded type, including integers, strings, dictionaries, and other lists.
// Example: l4:spam4:eggse represents the list of two strings: [ "spam", "eggs" ] 


#include "BencodedList.h"

#include <assert.h>

#include "BencodedInt.h"
#include "BencodedString.h"
#include "BencodedDictionary.h"



cBencodedList::cBencodedList()
{
}// END cBencodedList



cBencodedList::cBencodedList(const cBencodedList& rhs)
: cBencodedType(rhs)
{
	*this = rhs;
}// END cBencodedList



cBencodedList::~cBencodedList()
{
	Clear();
}// END ~cBencodedList



cBencodedType* cBencodedList::Clone()
{
	cBencodedList* blist = new cBencodedList(*this);
	return blist;
}// END Clone



const cBencodedList& cBencodedList::operator= (const cBencodedList& rhs)
{
	assert(typeid(rhs) == typeid(*this));
	assert(this != &rhs);

	assert(mList.empty());
	Clear();

	//cBencodedType::operator=(rhs);
	
	//mList = rhsList.mList;
	
	std::string rawdata = rhs.RawData();
	mRawData.clear();	
	Parse(rawdata.c_str(), rawdata.size());

	return *this;
}// END operator=



const cBencodedType* cBencodedList::GetElement(u32 index) const
{
	assert(index < mList.size());
	if(index >= mList.size())
	{
		return NULL;
	}

	return mList[index];
}// END GetElement



void cBencodedList::AddElement(cBencodedType* elem)
{
	cBencodedType* pValue;
	switch(elem->Type())
	{
	case BEN_INT:
		pValue = new cBencodedInt;
		*(static_cast<cBencodedInt*>(pValue)) = *(static_cast<const cBencodedInt*> (elem));
		break;
	case BEN_STRING:
		pValue = new cBencodedString;
		*(static_cast<cBencodedString*>(pValue)) = *(static_cast<const cBencodedString*> (elem));
		break;
	case BEN_LIST:
		pValue = new cBencodedList;
		*(static_cast<cBencodedList*>(pValue)) = *(static_cast<const cBencodedList*> (elem));
		break;

	case BEN_DICTIONARY:
		pValue = new cBencodedDictionary;
		*(static_cast<cBencodedDictionary*>(pValue)) = *(static_cast<const cBencodedDictionary*> (elem));
		break;

	case BEN_INVLAID:
	default:
		assert(0);
		return;
	}
	mList.push_back(pValue);
}// END AddElement



void cBencodedList::Clear()
{
	for(u32 i=0; i < mList.size(); i++)
	{
		delete mList[i];
		mList[i]=NULL;
	}
	mList.clear();
}// END Clear



bool cBencodedList::Parse(const char* data, u32 dataSize)
{
	Clear();

	bool parseResult = cBencodedType::Parse(data, dataSize);
	if(parseResult == false)
	{
		return false;
	}

	if(data[0] != 'l' && data[0] != 'L')
	{
		return false;
	}


	const char* elem = data + 1;			
	while(*elem != 'e' && *elem != 'E')
	{
		cBencodedType* pBt;

		switch(NextElementType(elem))
		{
		case BEN_INT:
			pBt = new cBencodedInt;
			break;

		case BEN_STRING:
			pBt = new cBencodedString;
			break;

			// lists can contain other lists but eventually the final one will boil down to a list of core string / int elements only
		case BEN_LIST:
			pBt = new cBencodedList;
			break;

		case BEN_DICTIONARY:
			pBt = new cBencodedDictionary;
			break;

		
		case BEN_INVLAID:
		default:
			assert(0);			
			delete pBt;
			Clear();
			return false;
		}

		if(!pBt->Parse(elem, dataSize - (elem - data)))
		{
			delete pBt;
			Clear();
			return false;
		}
		mList.push_back(pBt);
	
		s32 elementSize = ElementSizeInBytes(elem, dataSize - (elem - data));
		if(elementSize < 0)
		{
			return false;
		}
		elem += elementSize;
	}		


	return true;
}// END Parse



void cBencodedList::Write(const char* data)
{
	ClearRawData();

	mRawData += "l";
	for(u32 i=0; i < mList.size(); i++)
	{
		mRawData += mList[i]->RawData();
	}
	mRawData += "e";
}// END Write
