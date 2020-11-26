// Jon Bellamy


#include "BencodedType.h"

#include <stdio.h>
#include <assert.h>
#include <string>


cBencodedType::cBencodedType()
{
}// END cBencodedType



cBencodedType::cBencodedType(const cBencodedType& rhs)
{	
}// END cBencodedType



cBencodedType::~cBencodedType()
{
}// END ~cBencodedType



const cBencodedType& cBencodedType::operator=(const cBencodedType& rhs)
{
	// You are FUCKED if you get here, it should land at one of the derived type operator= methods.
	// PRobably the best solution to this is to use this method but call a virtual Clone/Copy method
	assert(0);
	return *this;
}// END operator=



bool cBencodedType::Parse(const char* data, u32 dataSize)
{
	s32 size = ElementSizeInBytes(data, dataSize);
	if(size <= 0)
	{
		return false;
	}
	mRawData.insert(0, data, size);
	return true;
}// END Parse



// look at the passed buffer and checks what type it is
cBencodedType::BenType cBencodedType::NextElementType(const char* data)
{
	if(data[0] == 'i' || data[0] == 'I')
	{
		return BEN_INT;
	}

	u32 length;
	if(sscanf(data, "%d:", &length) == 1)
	{
		return BEN_STRING;
	}

	if(data[0] == 'l' || data[0] == 'L')
	{
		return BEN_LIST;
	}

	if(data[0] == 'd' || data[0] == 'D')
	{
		return BEN_DICTIONARY;
	}

	return BEN_INVLAID;
}// END NextElementType



// -1 == invalid
s32 cBencodedType::ElementSizeInBytes(const char* data, u32 dataSize)
{
	switch(NextElementType(data))
	{
	case BEN_INT:
		{
			if(data[0] != 'i' && data[0] != 'I')
			{
				return -1;
			}

			u32 i=0;
			while(data[i] != 'e' && data[i] != 'E')
			{
				i++;

				if(i > dataSize)
				{
					return -1;
				}
			}
			// + 1 for the 'e'
			return i + 1;
		}

	case BEN_STRING:
		{
			u32 length;
			if(sscanf(data, "%d:", &length) != 1)
			{
				return -1;
			}

			std::string tmp(data);
			u32 index = static_cast<u32> (tmp.find(":"));
			if(index == std::string::npos)
			{
				return -1;
			}
			index++;

			if((index + length) > dataSize)
			{
				return -1;
			}

			return index + length;
		}

	// lists can contain other lists but eventually the final one will boil down to a list of core string / int elements only
	case BEN_LIST:
		{		
			const char* elem = data + 1;			
			while(*elem != 'e' && *elem != 'E')
			{
				s32 elementSize = ElementSizeInBytes(elem, dataSize - (elem - data));
				if(elementSize <= 0)
				{
					return -1;
				}
				elem += elementSize;

				if(elem > (data + dataSize))
				{
					return -1;
				}
			}		
			// + 1 for the 'e'
			return static_cast<u32>(elem - data) + 1;
		}


	case BEN_DICTIONARY:
		{
			s32 elementSize;
			const char* elem = data + 1;	
			while(*elem != 'e' && *elem != 'E')
			{
				// key
				elementSize = ElementSizeInBytes(elem, dataSize - (elem - data));
				if(elementSize <= 0)
				{
					return -1;
				}
				elem += elementSize;
				if(elem > (data + dataSize))
				{
					return -1;
				}

				// value
				elementSize = ElementSizeInBytes(elem, dataSize - (elem - data));
				if(elementSize <= 0)
				{
					return -1;
				}
				elem += elementSize;
				if(elem > (data + dataSize))
				{
					return -1;
				}
			}	
			// + 1 for the 'e'
			return static_cast<u32>(elem - data) + 1;
		}


	case BEN_INVLAID:
	default:
		return -1;
	}
}// END ElementSizeInBytes