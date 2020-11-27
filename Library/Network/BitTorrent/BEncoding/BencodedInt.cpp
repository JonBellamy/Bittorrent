// Jon Bellamy

// Integers are encoded as follows: i<integer encoded in base ten ASCII>e
// The initial i and trailing e are beginning and ending delimiters. You can have negative numbers such as i-3e. You cannot prefix the number with a zero such as i04e. However, i0e is valid.
// Example: i3e represents the integer "3" 
//  * NOTE: The maximum number of bit of this integer is unspecified, but to handle it as a signed 64bit integer is mandatory to handle "large files" aka .torrent for more that 4Gbyte 


#include "BencodedInt.h"

#include <stdio.h>
#include <assert.h>



cBencodedInt::cBencodedInt()
: mInt(0)
{
}// END cBencodedInt


cBencodedInt::cBencodedInt(const cBencodedInt& rhs)
: cBencodedType(rhs)
{
	*this = rhs;
}// END cBencodedInt


cBencodedInt::cBencodedInt(s64 i)
{
	mInt = i;
	Write(i);
}// END cBencodedInt



cBencodedInt::~cBencodedInt()
{
}// END ~cBencodedInt



cBencodedType* cBencodedInt::Clone()
{
	cBencodedInt* bint = new cBencodedInt(*this);
	return bint;
}// END Clone



const cBencodedInt& cBencodedInt::operator= (const cBencodedInt& rhs)
{
	//assert(typeid(rhs) == typeid(*this));
	assert(this != &rhs);
	const cBencodedInt& rhsInt = static_cast<const cBencodedInt&> (rhs);	

	//cBencodedType::operator=(rhs);
	mRawData = rhs.mRawData;
	mInt = rhsInt.mInt;
	return *this;
}// END operator=



bool cBencodedInt::Parse(const char* data, u32 dataSize)
{
	bool parseResult = cBencodedType::Parse(data, dataSize);
	if(parseResult == false)
	{
		return false;
	}

	if(sscanf(data, "i%I64de", &mInt) != 1)
	{
		return false;
	}
	return true;
}// END Parse



void cBencodedInt::Write(const char* data)
{
	ClearRawData();
	char sz[128];
	sprintf(sz, "i%se", data);
	mRawData = sz;
}// END Write



void cBencodedInt::Write(s64 i)
{
	char sz[128];
	sprintf(sz, "%I64d", i);
	Write(sz);
}// END Write


