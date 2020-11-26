
#ifndef BEN_DICT_H
#define BEN_DICT_H

#if USE_PCH
#include "stdafx.h"
#endif

#include "BencodedType.h"

#include <vector>


class cBencodedString;


class cBencodedDictionary : public cBencodedType
{
public:
    cBencodedDictionary();
	cBencodedDictionary(const cBencodedDictionary& rhs);
	~cBencodedDictionary();

	cBencodedType* Clone();

	const cBencodedDictionary& operator= (const cBencodedDictionary& rhs);

	virtual bool Parse(const char* data, u32 dataSize);
	virtual void Write(const char* data);

	virtual BenType Type() const;


	typedef struct tDictPair
	{
		tDictPair(const cBencodedString* key=NULL, const cBencodedType* val=NULL)	: pKey(key), pValue(val){}

		const cBencodedString* pKey;
		const cBencodedType* pValue;
	}DictPair;

	void AddElement(const DictPair& elem);

	// returns the value of the passed key
	const std::string* GetValueRawData(const std::string& key) const;

	const cBencodedType* GetValue(const std::string& key) const;

	bool KvPairExists(const std::string& key) const;

private:

	void Clear();

	std::vector<DictPair> mDictionary;
};





//////////////////////////////////////////////////////////////////////////
// inlines


inline cBencodedType::BenType cBencodedDictionary::Type() const
{
	return BEN_DICTIONARY;
}// END Type



/*
inline void cSegmentBuffer::Clear()
{
	mSegmentBuffer.clear();
}// END Clear
*/









#endif // BEN_DICT_H
