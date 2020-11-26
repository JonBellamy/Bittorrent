// Jon Bellamy 09/10/2009



#ifndef _SOAP_TYPES_H
#define _SOAP_TYPES_H


#include <string>

#include "Network/NetSettings.h"



namespace net 
{


// Will parse & convert a param or result string into its expected type. Note that many complex types are supported by SOAP such as dateTime, Base64Binary, arrays (eg xsd:string[2]).
// If we don't support a type (and there are plenty that we don't) we assert. Fix them up as we go
class cSoapType
{
public:
	cSoapType();
	cSoapType(const std::string& type, const std::string& val);
//	virtual ~cSoapType();	
	
	const cSoapType& operator=(const cSoapType& rhs);
	
	void Set(const std::string& type, const std::string& val);

	typedef enum
	{
		SOAP_TYPE_STRING=0,
		SOAP_TYPE_DECIMAL,
		SOAP_TYPE_INTEGER,
		SOAP_TYPE_FLOAT,
		SOAP_TYPE_DOUBLE,

		NUM_SUPPORTED_SOAP_TYPES,

		SOAP_TYPE_UNDEFINED
	}SoapType;


	SoapType Type() const { return mType; }

	// here are the functions to get the data
	const std::string AsString() const;
	s32 AsDecimal() const;
	s32 AsInteger() const;
	float AsFloat() const;
	double AsDouble() const;

private:
	std::string mValue;
	SoapType mType;
};



} // namespace net


#endif // _SOAP_TYPES_H