
#ifndef BEN_LIST_H
#define BEN_LIST_H

#if USE_PCH
#include "stdafx.h"
#endif

#include "BencodedType.h"

#include <vector>

class cBencodedList : public cBencodedType
{
public:
    cBencodedList();
	cBencodedList(const cBencodedList& rhs);
	~cBencodedList();

	cBencodedType* Clone();

	const cBencodedList& operator= (const cBencodedList& rhs);

	virtual bool Parse(const char* data, u32 dataSize);
	virtual void Write(const char* data);

	virtual BenType Type() const;

	u32 NumElements() const;

	const cBencodedType* GetElement(u32 index) const;
	void AddElement(cBencodedType* elem);


private:

	void Clear();

	std::vector<cBencodedType*> mList;
};





//////////////////////////////////////////////////////////////////////////
// inlines



inline cBencodedType::BenType cBencodedList::Type() const
{
	return BEN_LIST;
}// END Type


inline u32 cBencodedList::NumElements() const
{
	return static_cast<u32> (mList.size());
}// END NumElements









#endif // BEN_LIST_H
