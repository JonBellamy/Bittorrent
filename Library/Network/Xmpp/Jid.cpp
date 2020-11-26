// A valid JID contains a set of ordered elements formed of a domain identifier, node identifier, and resource identifier. 
// jid             = [ node "@" ] domain [ "/" resource ]
// domain          = fqdn / address-literal
// fqdn            = (sub-domain 1*("." sub-domain))
// sub-domain      = (internationalized domain label)
// address-literal = IPv4address / IPv6address


#include "Jid.h"

#include <assert.h>


namespace net {


cJid::cJid()
: mIsValid(false)
, mJid("")
, mDomain("")
, mNode("")
, mResource("")
{
}// END cJid



cJid::cJid(const char* szJid)
: mJid(szJid)
, mDomain("")
, mNode("")
, mResource("")
{
	mIsValid = Parse();
}// END cJid



cJid::cJid(const cJid& rhs)
{
	*this = rhs;
}// END cJid



const cJid& cJid::operator= (const cJid& rhs)
{
	mJid = rhs.mJid;
	mDomain = rhs.mDomain;
	mNode = rhs.mNode;
	mResource = rhs.mResource;
	mIsValid = rhs.mIsValid;
	return *this;
}// END operator=



bool cJid::operator== (const cJid& rhs) const
{
	return mJid == rhs.mJid;
}// END operator==



bool cJid::Parse()
{
	std::string::size_type index1, index2;
	
	// node
	index1 = mJid.find("@");
	if(index1 == std::string::npos)
	{
		return false;
	}
	mNode = mJid.substr(0, index1);

	// domain
	index2 = mJid.find("/", index1);
	if(index2 == std::string::npos)
	{		
		if(index1 == mJid.size()-1)
		{
			// no domain
			return false;
		}

		// partial jid ...

		// no resource but still a valid jid
		mDomain = mJid.substr(index1+1, mJid.size() - index1);
		return true;
	}
	
	// full jid ...

	// domain
	mDomain = mJid.substr(index1+1, index2 - index1 - 1);

	// resource
	mResource = mJid.substr(index2+1,  mJid.size() - index2);

	return true;
}// END Parse



} // namespace net

