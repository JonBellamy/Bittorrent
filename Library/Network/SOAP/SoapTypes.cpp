// Jon Bellamy 09/10/2009



#if USE_PCH
#include "stdafx.h"
#endif

#include <stdio.h>

#include "SoapTypes.h"
#include "Soap.h"


namespace net 
{


cSoapType::cSoapType()
: mValue("")
, mType(SOAP_TYPE_UNDEFINED)
{
}// END cSoapType



cSoapType::cSoapType(const std::string& type, const std::string& val)
: mValue(val)
{
	Set(type, val);
}// END cSoapType



const cSoapType& cSoapType::operator=(const cSoapType& rhs)
{
	mType = rhs.mType;
	mValue = rhs.mValue;
	return *this;
}// END operator=



void cSoapType::Set(const std::string& type, const std::string& val)
{
	mValue = val;

	// lookup what type we are dealing with
	bool found=false;
	for(int st=0; st < NUM_SUPPORTED_SOAP_TYPES; st++)
	{
		if(type == SUPPORTED_XSD_TYPES[st])
		{
			mType = static_cast<SoapType>(st);
			found=true;
			break;
		}
	}

	if(!found)
	{
		// unsupported soap type
		assert(0);
	}
}// END Set



//////////////////////////////////////////////////////////////////////////
// here are the functions to get the data ...


const std::string cSoapType::AsString() const
{
	// lets allow any type to be read back as a string
	return mValue;
}// END AsString



s32 cSoapType::AsDecimal() const
{
	// TODO : difference between this and int?
	assert(mType==SOAP_TYPE_DECIMAL);
	s32 v;
	if(sscanf_s(mValue.c_str(), "%d", &v, mValue.size()) != 1)
	{
		assert(0);
		return 0;
	}
	return v;
}// END AsDecimal



s32 cSoapType::AsInteger() const
{
	assert(mType==SOAP_TYPE_INTEGER);
	s32 v;
	if(sscanf_s(mValue.c_str(), "%d", &v, mValue.size()) != 1)
	{
		assert(0);
		return 0;
	}
	return v;
}// END AsInteger



float cSoapType::AsFloat() const
{
	assert(mType==SOAP_TYPE_FLOAT);
	float v;
	if(sscanf_s(mValue.c_str(), "%f", &v, mValue.size()) != 1)
	{
		assert(0);
		return 0;
	}
	return v;	
}// END AsFloat



double cSoapType::AsDouble() const
{
	assert(mType==SOAP_TYPE_DOUBLE);
	double v;
	if(sscanf_s(mValue.c_str(), "%f", &v, mValue.size()) != 1)
	{
		assert(0);
		return 0;
	}
	return v;
}// END AsDouble




} // namespace net