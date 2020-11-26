// Jon Bellamy 09/10/2009
// Used to call SOAP services via HTTP


#if USE_PCH
#include "stdafx.h"
#endif

#include "Soap.h"

#include <string>


// these follow cSoapType::SoapType
const char SUPPORTED_XSD_TYPES[net::cSoapType::NUM_SUPPORTED_SOAP_TYPES][16]  = { "xsd:string", "xsd:decimal", "xsd:integer", "xsd:float", "xsd:double"};


namespace net 
{


cSoapClient::cSoapClient()
: mCompleteCb(NULL)
, mCompleteParam(NULL)
{
}// END cSoapClient



cSoapClient::~cSoapClient()
{
}// END ~cSoapClient



void cSoapClient::Process()
{
	mHttpClient.Process();
}// END Process



void cSoapClient::SendSoapRequest(const cUrl& service, const std::string& soapActionTag, SoapResponseCb cb, void* pParam)
{
	mCompleteCb = cb;
	mCompleteParam = pParam;
	mHttpClient.SendSoapRequest(service, &mRequest, soapActionTag, net::STREAM_CONTENT_TO_BUFFER, HttpHeadCb, this, HttpContentCb, this);
}// END SendSoapRequest



void cSoapClient::HttpHeadCb(bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param)
{
	cSoapClient* pThis = reinterpret_cast<cSoapClient*>(param);
	if(!success)
	{
		if(pThis->mCompleteCb)
		{
			cSoapResponse r(cSoapResponse::SOAP_HTTP_FAIL);
			pThis->mCompleteCb(false, r, pThis->mCompleteParam);
		}
	}
}// END HttpHeadCb



void cSoapClient::HttpContentCb(bool success, const net::cHttpMessageHeader& request, const net::cHttpMessage& replyMessage, void* param)
{
	cSoapClient* pThis = reinterpret_cast<cSoapClient*>(param);
	
	cSoapResponse response;

	bool soapParseSuccess=false;

	if(success)
	{
		// TODO : failures are passed to here with different SOAP xml describing the problem. Need a way to parse and pass this back

		// this is easily stripped out if we don't want to use exceptions
		try
		{
			response.Parse(reinterpret_cast<const char*>(replyMessage.StreamedContentBuffer().Data()));			
			soapParseSuccess = true;
		}
		catch(cSoapResponse::SoapResult r)
		{
			switch(r)
			{
			case cSoapResponse::SOAP_PARSE_RESPONSE_FAIL:
				break;

			default:
				assert(0);
			}
		}
	}
	else
	{
		response.SetResult(cSoapResponse::SOAP_HTTP_FAIL);
	}


	if(pThis->mCompleteCb)
	{
		pThis->mCompleteCb(soapParseSuccess, response, pThis->mCompleteParam);
	}
}// END HttpContentCb



} // namespace net