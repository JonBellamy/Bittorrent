
#ifndef XML_STANZA_H
#define XML_STANZA_H

#include <string>

#include "Network/Xmpp/XmppValues.h"
#include "Network/Xmpp/Jid.h"
#include "TinyXml/tinyxml.h"


namespace net {



class cXmppStanza
{
public:
	
	cXmppStanza();
	virtual ~cXmppStanza() {}

	//cXmppStanza (const cXmppStanza& rhs) { *this = rhs; }
	const cXmppStanza& operator= (const cXmppStanza& rhs);
	virtual cXmppStanza* Clone() const { return NULL; }

private:
	bool operator== (const cXmppStanza& rhs) const;


public:

	virtual bool Parse(const std::string& strXml) { assert(0); return false; }

	virtual std::string AsString() const;

	bool HasId() const { return mId != INVALID_STANZA_ID; }
	u32 Id() const { return mId; }


protected:

	// advances the static id and returns it as a string
	std::string GenerateId();

	TiXmlDocument mXmlDoc;
	u32 mId;

private:

	enum
	{
		INVALID_STANZA_ID = 0
	};

	static u32 mNextId;
}; // cXmppStanza;



//////////////////////////////////////////////////////////////////////////


class cMessageStanza : public cXmppStanza
{
public:
	cMessageStanza();
	cMessageStanza(const cJid& jidTo);
	cMessageStanza (const cMessageStanza& rhs) { *this = rhs; }
	const cMessageStanza& operator= (const cMessageStanza& rhs);
	virtual cXmppStanza* Clone() { return new cMessageStanza(*this); }
	
	bool Parse(const std::string& strXml);

	void AddBodyNode(const char* szBodyText);

	void AddChatState(XmppChatState chatState);

	void AddSubjectNode(const char* szBodyText);
	void AddThreadNode(const char* szBodyText);

private:
	TiXmlElement* mMsgNode;
	TiXmlElement* mChatStateNode;
	TiXmlElement* mBodyNode;
	TiXmlElement* mThreadNode;

	// TODO : can have more than one subject
	TiXmlElement* mSubjectNode;						
}; // cMessageStanza



//////////////////////////////////////////////////////////////////////////


class cPresenceStanza : public cXmppStanza
{
public:
	cPresenceStanza();
	cPresenceStanza(const cJid* from, const cJid* to);
	cPresenceStanza (const cPresenceStanza& rhs) { *this = rhs; }
	const cPresenceStanza& operator= (const cPresenceStanza& rhs);
	virtual cXmppStanza* Clone() const { return new cPresenceStanza(*this); }

	bool Parse(const std::string& strXml);

	void AddShowNode(XmppPresenceShowStatus show);
	void AddStatusNode(const std::string& statusStr);

	cJid From() const;
	cJid To() const;
	XmppPresenceShowStatus ShowStatus() const;
	const TiXmlElement* PresenceRootNode() const { return mPresenceNode; }

private:
	TiXmlElement* mPresenceNode;
	TiXmlElement* mShowNode;
	TiXmlElement* mStatusNode;
}; // cPresenceStanza


//////////////////////////////////////////////////////////////////////////


class cIqStanza : public cXmppStanza
{
public:
	cIqStanza();
	cIqStanza (const cIqStanza& rhs) { *this = rhs; }
	const cIqStanza& operator= (const cIqStanza& rhs);
	virtual cXmppStanza* Clone() const { return new cIqStanza(*this); }

	bool Parse(const std::string& strXml);

	// the following functions format the IQ stanza

	// set's
	void BindResource(const char* szResourceName);
	void EstablishSession();

	// get's
	void GetRoster();

	// is this a result stanza
	bool IsResult() const;


	const TiXmlElement* IqRootNode() const { return mIqNode; }

private:

	void FormatAsGetQuery();

	bool mStanzaGenerated;
	TiXmlElement* mIqNode;
	TiXmlElement* mQueryNode;
}; // cIqStanza





//////////////////////////////////////////////////////////////////////////
// The following classes don't really represent stanza's per se but it is convenient to send all xml to the stream using the stanza base.
// They offer quick & managed access/creation of the various small pieces of xml we need for an xmpp stream.


// start stream xml
class cStreamStanza : public cXmppStanza
{
public:
	cStreamStanza();
	cStreamStanza(const char* to);

	virtual std::string AsString() const;

	bool Parse(const std::string& strXml);

	static std::string CloseStreamXml();

private:
	TiXmlElement* mStreamNode;
}; // cStreamStanza




class cStreamFeaturesStanza : public cXmppStanza
{
public:
	cStreamFeaturesStanza();
	bool Parse(const std::string& strXml);

	const TiXmlElement* RootNode() const { return mpFeaturesNode; }
	const TiXmlElement* MechanismsNode() const { return mpMechanismsNode; }

private:
	TiXmlElement* mpFeaturesNode;
	TiXmlElement* mpMechanismsNode;
};// cStreamFeaturesStanza;



class cStartTlsStanza : public cXmppStanza
{
public:
	cStartTlsStanza();
}; // cStartTlsStanza



class cSaslStanza : public cXmppStanza
{
public:
	typedef enum
	{
		PLAIN=0
	}SaslType;

	cSaslStanza(SaslType saslType, const std::string& username, const std::string& password);

	static bool ParseResponse(const std::string& strXml);

private:
	void BuildPlainSaslXml(const std::string& username, const std::string& password);
}; // cSaslStanza






} // namespace net


#endif // XML_STANZA_H