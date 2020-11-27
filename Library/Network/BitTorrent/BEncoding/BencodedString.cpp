// Jon Bellamy

// Byte strings are encoded as follows: <string length encoded in base ten ASCII>:<string data>
// Note that there is no constant beginning delimiter, and no ending delimiter.
// Example: 4:spam represents the string "spam" 


#include "BencodedString.h"

#include <assert.h>
#include <string.h>



cBencodedString::cBencodedString()
{
}// END cBencodedString



cBencodedString::cBencodedString(const char* szValue)
{
	mString = szValue;
	Write(szValue);
}// END cBencodedString



// used to insert binary data into a sting
cBencodedString::cBencodedString(const u8* data, u32 size)
{
	mString.insert(0, reinterpret_cast<const char*>(data), size);
	
	char sz[64];
	sprintf(sz, "%u:", size);
	mRawData += sz;
	mRawData.insert(mRawData.size(), reinterpret_cast<const char*>(data), size);
}// END cBencodedString



cBencodedString::cBencodedString(const cBencodedString& rhs)
: cBencodedType(rhs)
{
	*this = rhs;
}// END cBencodedString



cBencodedString::~cBencodedString()
{
}// END ~cBencodedString



cBencodedType* cBencodedString::Clone()
{
	cBencodedString* bstr = new cBencodedString(*this);
	return bstr;
}// END Clone



const cBencodedString& cBencodedString::operator= (const cBencodedString& rhs)
{
	//assert(typeid(rhs) == typeid(*this));
	assert(this != &rhs);
	const cBencodedString& rhsString = static_cast<const cBencodedString&> (rhs);

	//cBencodedType::operator=(rhsString);
	mRawData = rhs.mRawData;
	mString = rhsString.mString;

	//mString.clear();
	//Parse(mRawData.c_str(), mRawData.size());

	return *this;
}// END operator=



bool cBencodedString::Parse(const char* data, u32 dataSize)
{
	mString.clear();

	bool parseResult = cBencodedType::Parse(data, dataSize);
	if(parseResult == false)
	{
		return false;
	}

	u32 length;
	if(sscanf(data, "%d:", &length) != 1)
	{
		return false;
	}

	if(length > dataSize)
	{
		return false;
	}

	std::string tmp(data);
	u32 index = static_cast<u32> (tmp.find(":"));
	if(index == std::string::npos)
	{
		return false;
	}
	index++;

	mString.insert(0, data + index, length);
	return true;
}// END Parse



void cBencodedString::Write(const char* data)
{
	ClearRawData();
	u32 length = static_cast<u32>(strlen(data));
	char sz[64];
	sprintf(sz, "%u:", length);
	mRawData += sz;
	mRawData += data;
}// END Write
