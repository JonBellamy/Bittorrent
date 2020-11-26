// A valid JID contains a set of ordered elements formed of a domain identifier, node identifier, and resource identifier. 
// jid             = [ node "@" ] domain [ "/" resource ]
// domain          = fqdn / address-literal
// fqdn            = (sub-domain 1*("." sub-domain))
// sub-domain      = (internationalized domain label)
// address-literal = IPv4address / IPv6address


#ifndef JABBER_ID_H
#define JABBER_ID_H


#include <string>



namespace net {



class cJid
{
public:
	
	cJid();
	explicit cJid(const char* szJid);	
	cJid (const cJid& rhs);
	virtual ~cJid() {}
	const cJid& operator= (const cJid& rhs);
	bool operator== (const cJid& rhs) const;


	bool IsValid() const { return mIsValid; }

	bool IsFullJid() const { return (IsValid() && (mResource.size() > 0)); }

	const std::string& Domain() const;
	const std::string& Node() const;
	const std::string& Resource() const;

	const std::string& AsString() const { return mJid; }

private:

	bool Parse();

	bool mIsValid;
	std::string  mJid;

	std::string  mDomain;
	std::string  mNode;
	std::string  mResource;
};




} // namespace net


#endif // JABBER_ID_H