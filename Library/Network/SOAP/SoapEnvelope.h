// Jon Bellamy 09/10/2009


#ifndef _SOAP_ENV_H
#define _SOAP_ENV_H

#include <string>

#include "Network/soap/SoapTypes.h"
#include "TinyXml/tinyxml.h"
#include "Network/NetSettings.h"


namespace net 
{


class cSoapClient;


class cSoapRequest
{
public:	
	cSoapRequest();

private:
	const cSoapRequest& operator= (const cSoapRequest& rhs);

public:
	std::string AsString() const;

	void SetService(std::string service, std::string serviceNamespace);
	
	void AddParam(const std::string& paramName, int val);
	void AddParam(const std::string& paramName, float val);
	void AddParam(const std::string& paramName, const std::string& val);

private:

	u32 NumberOfNamespacesInParam(const std::string& param);
	std::string GetNamespaceInParam(const std::string& param, u32 namespaceNumber);
	std::string GetFinalNodeNameForParam(const std::string& param);
	
	// returns the parent tag to add the param to
	TiXmlElement* CreateParamTags(const std::string& qualifiedParamName);

	TiXmlDocument mXmlDoc;
	TiXmlDeclaration mXmlDeclarationNode;
	TiXmlElement mSoapEnvelopeNode;
	TiXmlElement mSoapBodyNode;
	TiXmlElement mSoapServiceName;
}; // cSoapRequest



class cSoapResponse
{
public:	
	typedef enum
	{
		SOAP_RESPONSE_OK=0,
		SOAP_HTTP_FAIL,
		SOAP_PARSE_RESPONSE_FAIL,				// can be thrown from the constructor

		//INVALID_ARGUMENT etc

		SOAP_UNKNOWN_ERROR
	}SoapResult;

	SoapResult Result() const { return mResult; }
	
	const cSoapType& ReturnValue() const { return mReturnValue; }
	
private:

	friend class net::cSoapClient;

	// only the cSoapClient has any business constructing or parsing this
	cSoapResponse();
	cSoapResponse(SoapResult r);
	void SetResult(SoapResult r) { mResult = r; }
	void Parse(const char* responseXml) throw(...);

	std::string TrimNamespace(const std::string& fullNodeNodeText);


	SoapResult mResult;

	TiXmlDocument xmlDoc;
	TiXmlElement* pEnvelopeElement;
	TiXmlElement* pBodyElement;
	TiXmlElement* pResponseElement;

	TiXmlElement* pReturnElement;
	std::string returnSoapType;
	std::string returnSoapValue;

	cSoapType mReturnValue;
}; // cSoapResponse



} // namespace net


#endif // _SOAP_ENV_H