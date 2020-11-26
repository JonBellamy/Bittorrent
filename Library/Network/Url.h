// Jon Bellamy 11/01/2007

// Url Syntax :

//	foo://username:password@example.com:8042/over/there/index.dtb;type=animal?name=ferret#nose
//  \ /   \________________/\_________/ \__/\_________/ \___/ \_/ \_________/ \_________/ \__/
//   |           |               |        |     |        |     |         |           |     |
//scheme     userinfo       hostname  port  path  filename extension parameter(s) query fragment


#ifndef URL_H
#define URL_H

#include <string>


namespace net
{


class cUrl
{
public:

	cUrl();
	cUrl(const char* szFullUrl);
	cUrl(const cUrl& url);

	const cUrl& operator= (const cUrl& rhs);
	bool operator== (const cUrl& rhs);

	void Clear();

	bool IsValid() const;

	const std::string& AsString() const { return mFullUrl; }


	const std::string& HostName() const { return mHostName; }
	const std::string& ResourceWithPath() const { return mResourceWithPath; }
	
	std::string ResPathOnly() const;
	std::string ResourceName() const;

	bool HasPort() const { return mPort != 0; }
	u16 Port() const { return mPort; }

	bool HasParameters() const;
	void AddParameter(std::string param, std::string val);
	void AddParameter(std::string param, s32 val);
	std::string GetParameterValue(const std::string& param);

	u32 NumberOfParams() const;
	void GetParamKeyValuePair(u32 paramNo, std::string& key, std::string& value) const;
	std::string GetAllParams() const;

	typedef enum
	{
		SCHEME_NOT_SPECIFIED =0,

		SCHEME_HTTP,
		SCHEME_HTTPS,
		SCHEME_FTP,
		SCHEME_UDP,
		SCHEME_INVALID
	}UrlScheme;

	UrlScheme Scheme() const { return mScheme; }
	
private:

	void SetScheme(const std::string& scheme);

	bool Parse();

	std::string::size_type NextControlCharIndex(const std::string& url, std::string::size_type startIndex);

	void EscapeString(std::string& str);

	bool mValid;

	std::string mFullUrl;

	UrlScheme mScheme;
	std::string mHostName;	
	std::string mResourceWithPath;
	u16 mPort;

	//std::string mUserInfo;
	//std::string mParameters;
	//std::string mQuueryFragment;
};




}// namespace net


#endif // URL_H

