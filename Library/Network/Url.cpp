// Jon Bellamy 11/01/2007

//     //<user>:<password>@<host>:<port>/<url-path>


#if USE_PCH
#include "stdafx.h"
#endif

#include "Url.h"

#include <assert.h>


namespace net
{


cUrl::cUrl()
: mFullUrl("")
, mValid(false)
, mScheme(SCHEME_INVALID)
, mPort(0)
{
}// END cUrl



cUrl::cUrl(const char* szFullUrl)
: mFullUrl(szFullUrl)
, mValid(false)
, mScheme(SCHEME_INVALID)
, mPort(0)
{
	Parse();
}// END cUrl



cUrl::cUrl(const cUrl& url)
{
	*this = url;
}



const cUrl& cUrl::operator= (const cUrl& rhs)
{
	mValid = rhs.mValid;
	mFullUrl = rhs.mFullUrl;
	mScheme = rhs.mScheme;
//	mUserInfo = rhs.mUserInfo;
	mHostName = rhs.mHostName;
	mPort = rhs.mPort;
	mResourceWithPath = rhs.mResourceWithPath;
//	mParameters = rhs.mParameters;
//	mQuueryFragment = rhs.mQuueryFragment;
	return *this;
}// END operator=



bool cUrl::operator== (const cUrl& rhs)
{
	return mFullUrl == rhs.mFullUrl;
}// END operator== 



void cUrl::Clear()
{
	mValid = false;
	mFullUrl.clear();
	mScheme = SCHEME_INVALID;
	mHostName.clear();	
	mResourceWithPath.clear();
	mPort = 0;
}// END Clear



bool cUrl::IsValid() const
{
	return mValid;
}// END IsValid



void cUrl::SetScheme(const std::string& scheme)
{
	if(scheme.empty())
	{
		mScheme = SCHEME_NOT_SPECIFIED;
		return;
	}

	if(scheme == "http")
	{
		mScheme = SCHEME_HTTP;
		return;
	}

	if(scheme == "https")
	{
		mScheme = SCHEME_HTTPS;
		return;
	}

	if(scheme == "ftp")
	{
		mScheme = SCHEME_FTP; 
		return;
	}

	if(scheme == "udp")
	{
		mScheme = SCHEME_UDP; 
		return;
	}

	mScheme = SCHEME_INVALID;
}// END SetScheme



// http://en.wikipedia.org/wiki/URI_scheme#Generic_syntax
// http://effbot.org/zone/effnews-1.htm
// http://torrent.fedoraproject.org:6969/announce
bool cUrl::Parse()
{
	//<user>:<password>@<host>:<port>/<url-path>

	// An HTTP URL takes the form:
	// http://<host>:<port>/<path>?<searchpart>


	std::string::size_type  indexStart;

	indexStart = mFullUrl.find("://");

	std::string scheme = mFullUrl.substr(0, indexStart);
	SetScheme(scheme);

	if(Scheme() == SCHEME_INVALID)
	{
		mValid = false;
		return false;
	}

	// +3 == ://
	indexStart +=3;

	std::string::size_type hostStartIndex, hostEndIndex;
	hostStartIndex = (indexStart);

	bool hasUserAndPass = (mFullUrl.find("@") != std::string::npos);
	if(hasUserAndPass)
	{
		hostStartIndex = static_cast<u32>(mFullUrl.find("@"));
		if(hostStartIndex == std::string::npos)
		{
			assert(0);
			mValid=false;
			return false;
		}
		hostStartIndex++;
		// TODO : user and password are at the beginning of the string after the scheme
		assert(0);
	}

	bool hasParameter = (mFullUrl.find("?", hostStartIndex) != std::string::npos);
	bool hasFragment = (mFullUrl.find("#", hostStartIndex) != std::string::npos);
	bool hasPort = (mFullUrl.find(":", hostStartIndex) != std::string::npos);
	bool hasResource = (mFullUrl.find("/", hostStartIndex) != std::string::npos);


	// ordering is important here, you must go right to left in the potential order of url elements so that we end up with the right indexEnd

	hostEndIndex = mFullUrl.size();

	if(hasFragment)
	{
		hostEndIndex = mFullUrl.find("#", hostStartIndex);
	}

	if (hasParameter)
	{
		hostEndIndex = mFullUrl.find("?", hostStartIndex);
	}

	if(hasResource)
	{
		std::string::size_type resourceStartIndex = mFullUrl.find("/", hostStartIndex);
		std::string::size_type resourceEndIndex = mFullUrl.size();
		std::string::size_type index = NextControlCharIndex(mFullUrl, hostStartIndex);
		if(index != std::string::npos)
		{
			resourceEndIndex = index;
		}
		mResourceWithPath = mFullUrl.substr(resourceStartIndex, resourceEndIndex - resourceStartIndex);

		hostEndIndex = resourceStartIndex;
	}

	if (hasPort)
	{
		hostEndIndex = mFullUrl.find(":", hostStartIndex);
		const char* sz = mFullUrl.c_str() + hostEndIndex;
		if(sscanf(sz, ":%d", &mPort) != 1)
		{
			mValid = false;
			return false;
		}
	}

	mHostName = mFullUrl.substr(hostStartIndex, hostEndIndex - hostStartIndex);


	mValid=true;
	return true;
}// END Parse



std::string::size_type cUrl::NextControlCharIndex(const std::string& url, std::string::size_type startIndex)
{
	for(std::string::size_type i = startIndex; i < url.size() - startIndex; i++)
	{
		if( url[i] == '#' ||
			url[i] == '?' ||
			url[i] == ':')
		{
			return i;
		}
	}
	return std::string::npos;
}// END NextControlCharIndex



std::string cUrl::ResourceName() const
{
	std::string::size_type  index;
	index = mResourceWithPath.rfind("/");
	std::string str = mResourceWithPath.substr(index+1, mResourceWithPath.size() - index);
	return str;
}// END ResourceName



std::string cUrl::ResPathOnly() const
{
	std::string::size_type  index;
	index = mResourceWithPath.rfind("/");
	std::string str = mResourceWithPath.substr(0, index);
	return str;
}// END ResPathOnly



bool cUrl::HasParameters() const
{
	return mFullUrl.find("?") != std::string::npos;
}// END HasParameters



void cUrl::AddParameter(std::string param, std::string val)
{
	EscapeString(param);
	EscapeString(val);

	if(!HasParameters())
	{
		mResourceWithPath += "?";
		mFullUrl += "?";
	}
	else
	{
		mResourceWithPath += "&";
		mFullUrl += "&";
	}

	mResourceWithPath += param;
	mResourceWithPath += "=";
	mResourceWithPath += val;

	mFullUrl += param;
	mFullUrl += "=";
	mFullUrl += val;
}// END AddParameter



void cUrl::AddParameter(std::string param, s32 val)
{
	char szNum[32];
	sprintf(szNum, "%d", val);
	AddParameter(param, szNum);
}// END AddParameter



std::string cUrl::GetParameterValue(const std::string& param)
{
	std::string key, value;
	u32 numParams = NumberOfParams();
	for(u32 i=0; i < numParams; i++)
	{
		GetParamKeyValuePair(i, key, value);

		if(key == param)
		{
			return value;
		}
	}
	return "";
}// END GetParameterValue



u32 cUrl::NumberOfParams() const
{
	if(!HasParameters())
	{
		return 0;
	}

	std::string::size_type start = mFullUrl.find("?");	

	u32 count=1;
	while((start = mFullUrl.find("&", start)) != std::string::npos)
	{
		start++;
		count++;
	}
	return count;
}// END NumberOfParams



void cUrl::GetParamKeyValuePair(u32 paramNo, std::string& key, std::string& value) const
{
	if(!HasParameters() || paramNo >= NumberOfParams())
	{
		key = value = "";
		return;
	}

	std::string::size_type start = mFullUrl.find("?");
	start++;
	
	if(paramNo == 0)
	{
		std::string::size_type keyStart = start;
		std::string::size_type keyEnd = mFullUrl.find("=", keyStart);

		std::string::size_type valStart = keyEnd+1;
		std::string::size_type valEnd = mFullUrl.find("&", valStart);
		if(valEnd == std::string::npos)
		{
			valEnd = mFullUrl.size();
		}

		key = mFullUrl.substr(keyStart, keyEnd - keyStart);
		value = mFullUrl.substr(valStart, valEnd - valStart);
	}
	else
	{
		u32 count=0;
		while(count < paramNo)
		{
			start = mFullUrl.find("&", start);
			start++;
			count++;
		}
		std::string::size_type keyStart = start;
		std::string::size_type keyEnd = mFullUrl.find("=", keyStart);

		std::string::size_type valStart = keyEnd+1;
		std::string::size_type valEnd = mFullUrl.find("&", valStart);
		if(valEnd == std::string::npos)
		{
			valEnd = mFullUrl.size();
		}

		key = mFullUrl.substr(keyStart, keyEnd - keyStart);
		value = mFullUrl.substr(valStart, valEnd - valStart);
	}
}// END GetParamKeyValuePair



// returns the parameter list not including the '?'
std::string cUrl::GetAllParams() const
{
	u32 numParams = NumberOfParams();
	if(numParams == 0)
	{
		return "";
	}

	std::string paramsStr, k, v;

	for(u32 i=0; i < numParams; i++)
	{
		GetParamKeyValuePair(i, k, v);
		paramsStr += k + "=" + v;

		if(i+1 < numParams)
		{
			paramsStr += "&";
		}
	}

	return paramsStr;
}// END GetAllParams



void cUrl::EscapeString(std::string& str)
{
	char acceptedChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_~";
	

	std::string outStr;
	for(u32 i=0; i < str.size(); i++)
	{
		bool found=false;
		for(u32 j=0; j < sizeof(acceptedChars)-1; j++)
		{
			if(str[i] == acceptedChars[j])
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			// invalid char, escape it
			char sz[16];
			u8 b = str[i];
			sprintf(sz, "%%%.2X", b);
			outStr += sz;
		}
		else
		{
			outStr += str[i];
		}
	}
	str = outStr;
}// END EscapeString


}// namespace net
