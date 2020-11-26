// Jon Bellamy 09/10/2009
// Used to call SOAP services via HTTP


/*
SOAP 1.1
The following is a sample SOAP 1.1 request and response. The placeholders shown need to be replaced with actual values.

			POST /WebServiceTest/Service.asmx HTTP/1.1
			Host: localhost
			Content-Type: text/xml; charset=utf-8
			Content-Length: length
			SOAPAction: "http://tempuri.org/HelloWorld"

			<?xml version="1.0" encoding="utf-8"?>
			<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
			<soap:Body>
			<HelloWorld xmlns="http://tempuri.org/" />
			</soap:Body>
			</soap:Envelope>

			HTTP/1.1 200 OK
			Content-Type: text/xml; charset=utf-8
			Content-Length: length

			<?xml version="1.0" encoding="utf-8"?>
			<soap:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">
			<soap:Body>
			<HelloWorldResponse xmlns="http://tempuri.org/">
			<HelloWorldResult>string</HelloWorldResult>
			</HelloWorldResponse>
			</soap:Body>
			</soap:Envelope>

SOAP 1.2
The following is a sample SOAP 1.2 request and response. The placeholders shown need to be replaced with actual values.

	  POST /WebServiceTest/Service.asmx HTTP/1.1
	  Host: localhost
	  Content-Type: application/soap+xml; charset=utf-8
	  Content-Length: length

	  <?xml version="1.0" encoding="utf-8"?>
	  <soap12:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap12="http://www.w3.org/2003/05/soap-envelope">
	  <soap12:Body>
	  <HelloWorld xmlns="http://tempuri.org/" />
	  </soap12:Body>
	  </soap12:Envelope>

	  HTTP/1.1 200 OK
	  Content-Type: application/soap+xml; charset=utf-8
	  Content-Length: length

	  <?xml version="1.0" encoding="utf-8"?>
	  <soap12:Envelope xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap12="http://www.w3.org/2003/05/soap-envelope">
	  <soap12:Body>
	  <HelloWorldResponse xmlns="http://tempuri.org/">
	  <HelloWorldResult>string</HelloWorldResult>
	  </HelloWorldResponse>
	  </soap12:Body>
	  </soap12:Envelope>



  and another example:

  POST /Supplier HTTP/1.1
  Host: www.somesupplier.com
  Content-Type: text/xml; charset="utf-8"
  Content-Length: nnnn
  SOAPAction: "Some-URI"  

  <SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"> 
     <SOAP-ENV:Body>
        <m:OrderItem xmlns:m="Some-URI">
        <RetailerID>557010</RetailerID>
        <ItemNumber>1050420459</ItemNumber>
        <ItemName>AMF Night Hawk Pearl M2</ItemName>
        <ItemDesc>Bowling Ball</ItemDesc>
        <OrderQuantity>100</OrderQuantity>
        <WholesalePrice>130.95</WholeSalePrice>
        <OrderDateTime>2000-06-19 10:09:56</OrderDateTime>
        </m:OrderItem>
     </SOAP-ENV:Body>  
  </SOAP-ENV:Envelope>
*/


/*
Test service i found:
http://www.soapclient.com/soapclient?fn=soapform&template=/clientform.html&soaptemplate=/soapresult.html&soapwsdl=http://soapclient.com/xml/soapresponder.wsdl

soapClient.SetService("Method1");
soapClient.AddParam("bstrParam1", "Jon");
soapClient.AddParam("bstrParam2", "B");
soapClient.SendSoapRequest(net::cUrl("http://soapclient.com/xml/soapresponder.wsdl"), SoapCb, NULL);
*/




#ifndef _SOAP_H
#define _SOAP_H


#include "Network/soap/SoapEnvelope.h"
#include "TinyXml/tinyxml.h"
#include "Network/NetSettings.h"
#include "Network/Url.h"
#include "Network/http/HttpClient.h"



#define UNINITALIZED_SOAP_PARAM "soap..uninitialized"

#define TEMP_DEV_NAMESPACE "http://tempuri.org/"

#define SOAP_ENVELOPE_NODE "SOAP-ENV:Envelope"
#define SOAP_BODY_NODE "SOAP-ENV:Body"

// These id's are standard and are appended to the local namespace, eg <SOAP-ENV:Body>
#define ENVELOPE_ID "Envelope"
#define BODY_ID "Body"

// Response is appended to the namespace and method name with no space eg <mns:Method1Response ...>
#define RESPONSE_ID "Response"


// See http://www.w3.org/TR/xmlschema-2/#built-in-datatypes
#define XSI_TYPE			"xsi:type"

// these follow cSoapType::SoapType
extern const char SUPPORTED_XSD_TYPES[net::cSoapType::NUM_SUPPORTED_SOAP_TYPES][16];


namespace net 
{


class cSoapClient
{
public:	
	cSoapClient();
	virtual ~cSoapClient();

	typedef void (*SoapResponseCb) (bool success, const cSoapResponse& soapResponse, void* pParam);
	void SendSoapRequest(const cUrl& service, const std::string& soapActionTag, SoapResponseCb cb, void* pParam);

	void Process();


	void SetService(std::string service, std::string serviceNamespace=TEMP_DEV_NAMESPACE) { mRequest.SetService(service, serviceNamespace); }
	void AddParam(const std::string& paramName, int val) { mRequest.AddParam(paramName, val); }
	void AddParam(const std::string& paramName, float val) { mRequest.AddParam(paramName, val); }
	void AddParam(const std::string& paramName, const std::string& val) { mRequest.AddParam(paramName, val); }

private:

	static void HttpHeadCb(bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param);
	static void HttpContentCb(bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param);

	enum
	{
		// needs to be big enough to hold all soap replies
		HTTP_BUFFER_SIZE = 4096
	};

	SoapResponseCb mCompleteCb;
	void* mCompleteParam;

	cSoapRequest mRequest;

	cHttpClient mHttpClient;
}; // cSoapClient



} // namespace net


#endif // _SOAP_H