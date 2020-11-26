
#ifndef BEN_INT_H
#define BEN_INT_H

#if USE_PCH
#include "stdafx.h"
#endif

#include "BencodedType.h"


class cBencodedInt : public cBencodedType
{
public:
    cBencodedInt();
	cBencodedInt(const cBencodedInt&);
	cBencodedInt(s64 i);
	~cBencodedInt();

	cBencodedType* Clone();

	const cBencodedInt& operator= (const cBencodedInt& rhs);

	virtual bool Parse(const char* data, u32 dataSize);
	virtual void Write(const char* data);	
	void Write(s64 i);

	virtual BenType Type() const;

	s64 Get() const;

	

private:

	s64 mInt;
};





//////////////////////////////////////////////////////////////////////////
// inlines


inline cBencodedType::BenType cBencodedInt::Type() const
{
	return BEN_INT;
}// END Type



inline s64 cBencodedInt::Get() const
{
	return mInt;
}// END Get










#endif // BEN_INT_H
