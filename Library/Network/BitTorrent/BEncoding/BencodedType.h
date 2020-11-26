
#ifndef BEN_TYPE_H
#define BEN_TYPE_H

#if USE_PCH
#include "stdafx.h"
#endif

#include <string>


class cBencodedType
{
public:
    cBencodedType();
	cBencodedType(const cBencodedType&);
	virtual ~cBencodedType();

	virtual cBencodedType* Clone() =0;
	const cBencodedType& operator= (const cBencodedType& rhs);

private:
	bool operator== (const cBencodedType& rhs);
	

public:
	virtual bool Parse(const char* data, u32 dataSize)=0;
	virtual void Write(const char* data)=0;

	typedef enum
	{
		BEN_STRING =0,
		BEN_INT,
		BEN_LIST,
		BEN_DICTIONARY,
		BEN_INVLAID
	}BenType;

	virtual BenType Type() const=0;

	// -1 == invalid
	static s32 ElementSizeInBytes(const char* data, u32 dataSize);

	// look at the passed buffer and checks what type it is
	static BenType NextElementType(const char* data);

	const std::string& RawData() const { return mRawData; }
	void ClearRawData() { mRawData = ""; }

protected:

	std::string mRawData;
};





//////////////////////////////////////////////////////////////////////////
// inlines

/*
inline void cSegmentBuffer::Clear()
{
	mSegmentBuffer.clear();
}// END Clear
*/









#endif // BEN_TYPE_H
