// Jon Bellamy

// Dictionaries are encoded as follows: d<bencoded string><bencoded element>e
// The initial d and trailing e are the beginning and ending delimiters. Note that the keys must be bencoded strings. The values may be any bencoded type, including integers, strings, lists, and other dictionaries. Keys must be strings and appear in sorted order (sorted as raw strings, not alphanumerics). The strings should be compared using a binary comparison, not a culture-specific "natural" comparison.

// Example: d3:cow3:moo4:spam4:eggse represents the dictionary { "cow" => "moo", "spam" => "eggs" }
// Example: d4:spaml1:a1:bee represents the dictionary { "spam" => [ "a", "b" ] }
// Example: d9:publisher3:bob17:publisher-webpage15:www.example.com18:publisher.location4:homee represents { "publisher" => "bob", "publisher-webpage" => "www.example.com", "publisher.location" => "home" } 


#include "BencodedDictionary.h"


#include <assert.h>

#include "BencodedInt.h"
#include "BencodedString.h"
#include "BencodedList.h"



cBencodedDictionary::cBencodedDictionary()
{
}// END cBencodedDictionary



cBencodedDictionary::cBencodedDictionary(const cBencodedDictionary& rhs)
: cBencodedType(rhs)
{
	*this = rhs;
}// END cBencodedDictionary



cBencodedDictionary::~cBencodedDictionary()
{
	Clear();
}// END ~cBencodedDictionary



cBencodedType* cBencodedDictionary::Clone()
{
	cBencodedDictionary* dict = new cBencodedDictionary(*this);
	return dict;
}// END Clone



const cBencodedDictionary& cBencodedDictionary::operator= (const cBencodedDictionary& rhs)
{
	assert(typeid(rhs) == typeid(*this));
	assert(this != &rhs);
	const cBencodedDictionary& rhsDict = static_cast<const cBencodedDictionary&> (rhs);

	//cBencodedType::operator=(rhs);
	//mDictionary = rhsDict.mDictionary;]
	
	assert(mDictionary.empty());
	Clear();

	std::string rawdata = rhs.RawData();
	mRawData.clear();
	mDictionary.clear();
	Parse(rawdata.c_str(), rawdata.size());
	
	return *this;
}// END operator=



void cBencodedDictionary::Clear()
{
	for(u32 i=0; i < mDictionary.size(); i++)
	{
		delete mDictionary[i].pKey;
		mDictionary[i].pKey=NULL;

		delete mDictionary[i].pValue;
		mDictionary[i].pValue=NULL;
	}
	mDictionary.clear();
}// END Clear



void cBencodedDictionary::AddElement(const DictPair& elem)
{
	cBencodedString* pKey = new cBencodedString(*(elem.pKey));
	cBencodedType* pValue;
	
	// Some rather nasty casting going on here to force the proper operator= usage
	// TODO: sort the operator= issue
	switch(elem.pValue->Type())
	{
	case BEN_INT:
		pValue = new cBencodedInt;
		*(static_cast<cBencodedInt*>(pValue)) = *(static_cast<const cBencodedInt*> (elem.pValue));
		break;
	case BEN_STRING:
		pValue = new cBencodedString;
		*(static_cast<cBencodedString*>(pValue)) = *(static_cast<const cBencodedString*> (elem.pValue));
		break;
	case BEN_LIST:
		pValue = new cBencodedList;
		*(static_cast<cBencodedList*>(pValue)) = *(static_cast<const cBencodedList*> (elem.pValue));
		break;
	case BEN_DICTIONARY:
		pValue = new cBencodedDictionary;
		*(static_cast<cBencodedDictionary*>(pValue)) = *(static_cast<const cBencodedDictionary*> (elem.pValue));
		break;
	case BEN_INVLAID:
	default:
		assert(0);
		return;
	}
	mDictionary.push_back(DictPair(pKey, pValue));
}// END AddElement



// returns the value of the passed key
const std::string* cBencodedDictionary::GetValueRawData(const std::string& key) const
{
	for(u32 i=0; i < mDictionary.size(); i++)
	{
		const cBencodedString* pKey = mDictionary[i].pKey;
		if(key == pKey->Get())
		{
			return &(mDictionary[i].pValue->RawData());
		}
	}
	return NULL;
}// END GetValueRawData



const cBencodedType* cBencodedDictionary::GetValue(const std::string& key) const
{
	for(u32 i=0; i < mDictionary.size(); i++)
	{
		const cBencodedString* pKey = mDictionary[i].pKey;
		if(key == pKey->Get())
		{
			return mDictionary[i].pValue;
		}
	}
	return NULL;
}// END GetValue



bool cBencodedDictionary::KvPairExists(const std::string& key) const
{
	return GetValue(key) != NULL;
}// END KvPairExists



bool cBencodedDictionary::Parse(const char* data, u32 dataSize)
{
	Clear();

	bool parseResult = cBencodedType::Parse(data, dataSize);
	if(parseResult == false)
	{
		return false;
	}

	if(data[0] != 'd' && data[0] != 'D')
	{
		return false;
	}

	s32 elementSize;

	const char* elem = data + 1;			
	while(*elem != 'e' && *elem != 'E')
	{
		DictPair dictPair;

		// key must be a string
		if(NextElementType(elem) == BEN_STRING)
		{
			cBencodedString* pBeStr = new cBencodedString;
			if(!pBeStr->Parse(elem, dataSize - (elem - data)))
			{
				assert(0);
				Clear();
				return false;
			}
			dictPair.pKey = pBeStr;	
			elementSize = ElementSizeInBytes(elem, dataSize - (elem - data));
			elem += elementSize;
		}
		else
		{
			assert(0);
			Clear();
			return false;
		}
	
		// value

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
		dictPair.pValue = pBt;
		mDictionary.push_back(dictPair);

		elementSize = ElementSizeInBytes(elem, dataSize - (elem - data));
		if(elementSize < 0)
		{
			return false;
		}
		elem += elementSize;
	}		


	return true;
}// END Parse



void cBencodedDictionary::Write(const char* data)
{
	ClearRawData();
	mRawData += "d";
	for(u32 i=0; i < mDictionary.size(); i++)
	{
		mRawData += mDictionary[i].pKey->RawData();
		mRawData += mDictionary[i].pValue->RawData();
	}
	mRawData += "e";
}// END Write