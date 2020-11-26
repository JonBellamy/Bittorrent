
#ifndef BEN_STRING_H
#define BEN_STRING_H

#if USE_PCH
#include "stdafx.h"
#endif

#include "BencodedType.h"

#include <string>

class cBencodedString : public cBencodedType
{
public:
    cBencodedString();
	cBencodedString(const char* szValue);
	cBencodedString(const u8* data, u32 size);
	cBencodedString(const cBencodedString& rhs);
	~cBencodedString();

	cBencodedType* Clone();

	const cBencodedString& operator= (const cBencodedString& rhs);

	virtual bool Parse(const char* data, u32 dataSize);
	virtual void Write(const char* data);

	virtual BenType Type() const;

	const std::string& Get() const;

private:

	std::string mString;
};





//////////////////////////////////////////////////////////////////////////
// inlines


inline cBencodedType::BenType cBencodedString::Type() const
{
	return BEN_STRING;
}// END Type


inline const std::string& cBencodedString::Get() const
{
	return mString;
}// END Get










#endif // BEN_STRING_H
