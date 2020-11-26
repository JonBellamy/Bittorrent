
#include "Stanza.h"

#include <assert.h>


#include "General/Rand.h"
#include "Network/Xmpp/XmppValues.h"
#include "Network/Base64.h"
#include "Network/bytestream.h"



namespace net {


u32 cXmppStanza::mNextId = 0xFF000000;


cXmppStanza::cXmppStanza()
: mId(INVALID_STANZA_ID)
{	
}// END cXmppStanza



const cXmppStanza& cXmppStanza::operator= (const cXmppStanza& rhs)
{
	mXmlDoc = rhs.mXmlDoc;
	mId = rhs.mId;
	return *this;
}// END operator=



// advances the static id and returns it as a string
std::string cXmppStanza::GenerateId()
{	
	assert(mId == INVALID_STANZA_ID);

	char szId[32];
	sprintf(szId, "%u", mNextId);
	mId = mNextId;
	mNextId++;
	return std::string(szId);
}// END GenerateId



std::string cXmppStanza::AsString() const
{
	TiXmlPrinter printer;
	mXmlDoc.Accept(&printer);
	return printer.CStr();
}// END AsString




//////////////////////////////////////////////////////////////////////////
// Message Stanza


// The <message/> stanza kind can be seen as a "push" mechanism whereby one entity pushes information to another entity, 
// similar to the communications that occur in a system such as email. All message stanzas SHOULD possess a 'to' 
// attribute that specifies the intended recipient of the message; upon receiving such a stanza, a server SHOULD route or 
// deliver it to the intended recipient (see Server Rules for Handling XML Stanzas (Server Rules for Handling XML Stanzas) 
// for general routing and delivery rules related to XML stanzas). 

cMessageStanza::cMessageStanza()
: mMsgNode(NULL)
, mChatStateNode(NULL)
, mBodyNode(NULL)
, mThreadNode(NULL)
, mSubjectNode(NULL)
{
}// END cMessageStanza



cMessageStanza::cMessageStanza(const cJid& jidTo)
: mMsgNode(NULL)
, mChatStateNode(NULL)
, mBodyNode(NULL)
, mThreadNode(NULL)
, mSubjectNode(NULL)
{
	/*
	<message type="chat" to="jonb_rs1@jabberd.eu" id="myid4">
		<body>hello world</body>
		<active xmlns="http://jabber.org/protocol/chatstates" />			// <<-- chat state
	</message>
	*/

	TiXmlElement msgNode(STANZA_MESSAGE);
	msgNode.SetAttribute(ATTRIBUTE_TYPE, MESSAGE_STANZA_TYPE_CHAT);
	msgNode.SetAttribute(ATTRIBUTE_TO, jidTo.AsString().c_str());
	msgNode.SetAttribute(ATTRIBUTE_ID, GenerateId().c_str());


	// add the elements to the doc and save 
	mMsgNode = static_cast<TiXmlElement*> (mXmlDoc.InsertEndChild(msgNode));
}// END cMessageStanza



const cMessageStanza& cMessageStanza::operator= (const cMessageStanza& rhs)
{
	cXmppStanza::operator =(rhs);
	mMsgNode = rhs.mMsgNode;
	mChatStateNode = rhs.mChatStateNode;
	mBodyNode = rhs.mBodyNode;
	mThreadNode = rhs.mThreadNode;
	mSubjectNode = rhs.mSubjectNode;	
	return *this;
}// END operator=



bool cMessageStanza::Parse(const std::string& strXml)
{
	assert(0);
	return false;
}// END Parse



void cMessageStanza::AddBodyNode(const char* szBodyText)
{
	TiXmlText bodyText(szBodyText);

	TiXmlElement bodyNode(MESSAGE_STANZA_BODY);
	bodyNode.InsertEndChild(bodyText);

	mBodyNode = static_cast<TiXmlElement*> (mMsgNode->InsertEndChild(bodyNode));
}// END AddBodyNode



void cMessageStanza::AddChatState(XmppChatState chatState)
{
	std::string str;
	switch(chatState)
	{
	case CHATSTATE_ACTIVE:
		str = MESSAGE_STANZA_CHATSTATE_ACTIVE;
		break;
	case CHATSTATE_INACTIVE:
		str = MESSAGE_STANZA_CHATSTATE_INACTIVE;
		break;
	case CHATSTATE_GONE:
		str = MESSAGE_STANZA_CHATSTATE_GONE;
		break;
	case CHATSTATE_COMPOSING:
		str = MESSAGE_STANZA_CHATSTATE_COMPOSING;
		break;
	case CHATSTATE_PAUSED:
		str = MESSAGE_STANZA_CHATSTATE_PAUSED;
		break;
	default:
		assert(0);
	}

	TiXmlElement chatStateNode(STANZA_MESSAGE);
	chatStateNode.SetAttribute(NS_XML, NS_VAL_CHAT_STATES);
	chatStateNode.SetValue(str.c_str());

	mChatStateNode = static_cast<TiXmlElement*> (mMsgNode->InsertEndChild(chatStateNode));
}// END AddChatState



void cMessageStanza::AddSubjectNode(const char* szBodyText)
{
	// TODO 
	assert(0);
}// END AddSubjectNode



void cMessageStanza::AddThreadNode(const char* szBodyText)
{
	// TODO 
	assert(0);
}// END AddThreadNode





//////////////////////////////////////////////////////////////////////////
// Presence Stanza


cPresenceStanza::cPresenceStanza()
: mPresenceNode(NULL)
, mShowNode(NULL)
, mStatusNode(NULL)
{
}// END cPresenceStanza



cPresenceStanza::cPresenceStanza(const cJid* from, const cJid* to)
: mPresenceNode(NULL)
, mShowNode(NULL)
, mStatusNode(NULL)
{
	assert(from);
	if(from)
	{
		TiXmlElement  presenceNode(STANZA_PRESENCE);
		presenceNode.SetAttribute(ATTRIBUTE_FROM, from->AsString().c_str());
		mPresenceNode = static_cast<TiXmlElement*> (mXmlDoc.InsertEndChild(presenceNode));
	}

	if(to)
	{
		mPresenceNode->SetAttribute(ATTRIBUTE_TO, to->AsString().c_str());
	}
}// END cPresenceStanza



const cPresenceStanza& cPresenceStanza::operator= (const cPresenceStanza& rhs)
{
	cXmppStanza::operator =(rhs);
	mPresenceNode = rhs.mPresenceNode;
	mShowNode = rhs.mShowNode;
	mStatusNode = rhs.mStatusNode;
	return *this;
}// END operator=



cJid cPresenceStanza::From() const
{
	if(mPresenceNode == NULL)
	{
		assert(0);
		return cJid();
	}
	const char* from = mPresenceNode->Attribute(ATTRIBUTE_FROM);
	if(from)
	{
		return cJid(from);
	}
	return cJid();
}// END From



cJid cPresenceStanza::To() const
{
	if(mPresenceNode == NULL)
	{
		assert(0);
		return cJid();
	}
	const char* to = mPresenceNode->Attribute(ATTRIBUTE_TO);
	if(to)
	{
		return cJid(to);
	}
	return cJid();
}// END To



XmppPresenceShowStatus cPresenceStanza::ShowStatus() const
{
	XmppPresenceShowStatus status=PRESENCE_INVALID;
	if(mShowNode && mShowNode->FirstChild())
	{
		std::string strShow(mShowNode->FirstChild()->Value());

		if(strShow == PRESENCE_STANZA_SHOW_AWAY) return PRESENCE_SHOW_AWAY;
		else if(strShow == PRESENCE_STANZA_SHOW_CHAT) return PRESENCE_SHOW_CHAT;
		else if(strShow == PRESENCE_STANZA_SHOW_DND) return PRESENCE_SHOW_DND;
		else if(strShow == PRESENCE_STANZA_SHOW_XA) return PRESENCE_SHOW_XA;
		else
		{
			assert(0);
		}
	}
	return status;
}// END ShowStatus


bool cPresenceStanza::Parse(const std::string& strXml)
{
/*
	<presence from='jonb_rs2@jabberd.eu/Test Resource' to='jonb_rs2@jabberd.eu/Test Resource'>
		<show>chat</show>
		<status>My status string</status>
	</presence>
*/

	mPresenceNode = NULL;
	mShowNode = NULL;
	mStatusNode = NULL;

	mXmlDoc.Clear();
	mXmlDoc.Parse(strXml.c_str());
	bool isError = mXmlDoc.Error();

	if(isError)
	{
		assert(0);
		return false;
	}

	mPresenceNode = mXmlDoc.RootElement();
	if( mPresenceNode == NULL ||
		mPresenceNode->Value() != std::string(STANZA_PRESENCE))
	{
		assert(0);
		return false;
	}

	mShowNode = mPresenceNode->FirstChildElement(PRESENCE_STANZA_SHOW);
	mStatusNode = mPresenceNode->FirstChildElement(PRESENCE_STANZA_STATUS);

	return true;
}// END Parse



void cPresenceStanza::AddShowNode(XmppPresenceShowStatus show)
{
	assert(mPresenceNode);
	std::string showStr;
	switch(show)
	{
	case PRESENCE_SHOW_AWAY:
		showStr = PRESENCE_STANZA_SHOW_AWAY;
		break;
	case PRESENCE_SHOW_CHAT:
		showStr = PRESENCE_STANZA_SHOW_CHAT;
		break;
	case PRESENCE_SHOW_DND:
		showStr = PRESENCE_STANZA_SHOW_DND;
		break;
	case PRESENCE_SHOW_XA:
		showStr = PRESENCE_STANZA_SHOW_XA;
		break;
	default:
		assert(0);
	}

	TiXmlElement showNode(PRESENCE_STANZA_SHOW);
	mShowNode = static_cast<TiXmlElement*> (mPresenceNode->InsertEndChild(showNode));

	TiXmlText showText(showStr.c_str());
	mShowNode->InsertEndChild(showText);
}// END AddShowNode



void cPresenceStanza::AddStatusNode(const std::string& statusStr)
{
	TiXmlElement statusNode(PRESENCE_STANZA_STATUS);
	mStatusNode = static_cast<TiXmlElement*> (mPresenceNode->InsertEndChild(statusNode));

	TiXmlText statusText(statusStr.c_str());
	mStatusNode->InsertEndChild(statusText);
}// END AddStatusNode




//////////////////////////////////////////////////////////////////////////
// Iq Stanza


cIqStanza::cIqStanza()
: mStanzaGenerated(false)
, mIqNode(NULL)
, mQueryNode(NULL)
{
	TiXmlElement iqNode(STANZA_IQ);	
	mIqNode = static_cast<TiXmlElement*> (mXmlDoc.InsertEndChild(iqNode));
}// END cIqStanza



const cIqStanza& cIqStanza::operator= (const cIqStanza& rhs)
{
	cXmppStanza::operator =(rhs);
	mStanzaGenerated = rhs.mStanzaGenerated;
	mIqNode = rhs.mIqNode;
	mQueryNode = rhs.mQueryNode;
	return *this;
}// END operator=



bool cIqStanza::Parse(const std::string& strXml)
{
	mIqNode = NULL;
	mQueryNode = NULL;
	mStanzaGenerated=true;

	mXmlDoc.Clear();
	mXmlDoc.Parse(strXml.c_str());
	bool isError = mXmlDoc.Error();

	if(isError)
	{
		assert(0);
		return false;
	}

	// as a general parse of an IQ stanza the only thing to really do is find the IQ node...
	mIqNode = mXmlDoc.RootElement();
	if( mIqNode == NULL ||
		mIqNode->Value() != std::string(STANZA_IQ))
	{
		assert(0);
		return false;
	}

	u32 id;
	const char* szId = mIqNode->Attribute(ATTRIBUTE_ID);
	if(szId == NULL ||
	   sscanf(szId, "%u", &id) != 1)
	{
		assert(0);
		return false;
	}
		
	mId = id;

	return true;
}// END Parse



void cIqStanza::FormatAsGetQuery()
{
//	<iq type="get" id="myid3">
//		<query xmlns="......" />
//	</iq>
	assert(!mStanzaGenerated);
	mStanzaGenerated=true;

	mIqNode->SetAttribute(ATTRIBUTE_ID, GenerateId().c_str());
	mIqNode->SetAttribute(ATTRIBUTE_TYPE, IQ_STANZA_TYPE_GET);

	TiXmlElement queryNode(IQ_STANZA_QUERY);
	mQueryNode = static_cast<TiXmlElement*> (mIqNode->InsertEndChild(queryNode));
}// END FormatAsGetQuery



void cIqStanza::BindResource(const char* szResourceName)
{
//	<iq type="set" id="myid1">
//		<bind xmlns="urn:ietf:params:xml:ns:xmpp-bind">
//			<resource>MyResource</resource>
//		</bind>
//	</iq>
	assert(!mStanzaGenerated);
	mStanzaGenerated=true;

	mIqNode->SetAttribute(ATTRIBUTE_ID, GenerateId().c_str());
	mIqNode->SetAttribute(ATTRIBUTE_TYPE, IQ_STANZA_TYPE_SET);

	TiXmlElement bindNode(IQ_STANZA_BIND);
	bindNode.SetAttribute(NS_XML, NS_VAL_BIND);	

	TiXmlElement resourceNode(IQ_STANZA_RESOURCE);
	TiXmlText resourceText(szResourceName);
	resourceNode.InsertEndChild(resourceText);

	bindNode.InsertEndChild(resourceNode);
	mIqNode->InsertEndChild(bindNode);
}// END BindResource



void cIqStanza::EstablishSession()
{
//	<iq type="set" id="myid2">
//		<session xmlns="urn:ietf:params:xml:ns:xmpp-session" />
//	</iq>
	assert(!mStanzaGenerated);
	mStanzaGenerated=true;

	mIqNode->SetAttribute(ATTRIBUTE_ID, GenerateId().c_str());
	mIqNode->SetAttribute(ATTRIBUTE_TYPE, IQ_STANZA_TYPE_SET);
	
	TiXmlElement sessionNode(IQ_STANZA_SESSION);
	sessionNode.SetAttribute(NS_XML, NS_VAL_SESSION);
	mIqNode->InsertEndChild(sessionNode);
}// END EstablishSession




void cIqStanza::GetRoster()
{
//	<iq type="get" id="myid3">
//		<query xmlns="jabber:iq:roster" />
//	</iq>
	FormatAsGetQuery();
	mQueryNode->SetAttribute(NS_XML, NS_QUERY_ROSTER);
}// END GetRoster



// is this a result stanza
bool cIqStanza::IsResult() const
{
	if(mIqNode == NULL)
	{
		assert(0);
		return false;
	}
	const char* xmlType = mIqNode->Attribute(ATTRIBUTE_TYPE);
	return std::string(xmlType) == IQ_STANZA_TYPE_RESULT;
}// END IsResult




//////////////////////////////////////////////////////////////////////////
// Miscellaneous Stanza's


cStreamStanza::cStreamStanza()
: mStreamNode(NULL)
{
	TiXmlDeclaration xmlDeclarationNode("1.0", "", "");	// <?xml version='1.0'?>
	mXmlDoc.InsertEndChild(xmlDeclarationNode);
}// END cStreamStanza



cStreamStanza::cStreamStanza(const char* to)
: mStreamNode(NULL)
{
	TiXmlDeclaration xmlDeclarationNode("1.0", "", "");	// <?xml version='1.0'?>
	mXmlDoc.InsertEndChild(xmlDeclarationNode);	

	
	// <?xml version='1.0'?>
	// <stream:stream
	// to='example.com'
	// xmlns='jabber:client'
	// xmlns:stream='http://etherx.jabber.org/streams'
	// version='1.0'>
	
	TiXmlElement streamNode(NODE_STREAM);
	streamNode.SetAttribute(ATTRIBUTE_TO, to);
	streamNode.SetAttribute(NS_XML, NS_VAL_JABBER_CLIENT);
	streamNode.SetAttribute(NS_STREAM, NS_VAL_STREAM);
	streamNode.SetAttribute(ATTRIBUTE_VERSION_KEY, ATTRIBUTE_VERSION_VAL);
	mStreamNode = static_cast<TiXmlElement *> (mXmlDoc.InsertEndChild(streamNode));
}// END cStreamStanza



bool cStreamStanza::Parse(const std::string& strXml)
{
	const char* szXml = strXml.c_str();

	bool hasXmlDeclaration = strXml[0] == '<' && strXml[1] == '?';

	if(hasXmlDeclaration)
	{
		TiXmlDeclaration xmlDeclarationNode;
		szXml = xmlDeclarationNode.Parse(szXml, NULL, TIXML_DEFAULT_ENCODING);
	}

	TiXmlElement streamElement("");
	streamElement.Parse(szXml, NULL, TIXML_DEFAULT_ENCODING);

	const std::string xmlNs = streamElement.Attribute(NS_XML);
	const std::string streamNs = streamElement.Attribute(NS_STREAM);
	const std::string version = streamElement.Attribute(ATTRIBUTE_VERSION_KEY);
	
	// check all required attributes are present...
	if(xmlNs.size() == 0 || streamNs.size() == 0 || version.size() == 0)
	{
		assert(0);
		return false;
	}

	// and that their values are correct
	if( xmlNs != NS_VAL_JABBER_CLIENT ||
		streamNs != NS_VAL_STREAM ||
		version != ATTRIBUTE_VERSION_VAL)
	{
		assert(0);
		return false;
	}

	//*mStreamNode = streamElement;
	return true;
}// END Parse



// when we pass back this xml we have to removed the closing / of the stream node otherwise the stream/xml is considered closed
std::string cStreamStanza::AsString() const
{
	std::string xml = cXmppStanza::AsString();
	std::string::size_type index = xml.rfind('/', xml.size());
	if(index == std::string::npos)
	{
		assert(0);
		return "";
	}
	xml.erase(index, 1);
	return xml;
}// END AsString



std::string cStreamStanza::CloseStreamXml()
{
	return "</stream:stream>";
}// END CloseStreamXml



cStreamFeaturesStanza::cStreamFeaturesStanza()
{
}// END cStreamFeaturesStanza



bool cStreamFeaturesStanza::Parse(const std::string& strXml)
{
/*
	<stream:features>
		<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>
		<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
			<mechanism>DIGEST-MD5</mechanism>
			<mechanism>PLAIN</mechanism>
		</mechanisms>
		<register xmlns='http://jabber.org/features/iq-register'/>
	</stream:features>
*/
	mXmlDoc.Parse(strXml.c_str());
	bool hasError = mXmlDoc.Error();
	if(hasError)
	{
		return false;
	}

	mpFeaturesNode = mXmlDoc.RootElement();

	if( mpFeaturesNode == NULL ||
		mpFeaturesNode->Value() != std::string(NODE_STREAM_FEATURES))
	{
		return false;
	}

	mpMechanismsNode = mpFeaturesNode->FirstChildElement(NODE_MECHANISMS);
	if(mpMechanismsNode == NULL)
	{
		return false;
	}
	const char* szXmlNs = mpMechanismsNode->Attribute(NS_XML);
	if(szXmlNs == NULL ||
		szXmlNs != std::string(NS_VAL_SASL))
	{
		return false;
	}



	return true;
}// END Parse




cStartTlsStanza::cStartTlsStanza()
{
	//<starttls xmlns="urn:ietf:params:xml:ns:xmpp-tls"/>
	TiXmlElement node(NODE_START_TLS);
	node.SetAttribute(NS_XML, NS_VAL_TLS);
	mXmlDoc.InsertEndChild(node);
}// END cStartTlsStanza



cSaslStanza::cSaslStanza(SaslType saslType, const std::string& username, const std::string& password)
{
	// for now, only supporting plain
	assert(saslType == PLAIN);
	BuildPlainSaslXml(username, password);
}// END cSaslStanza



void cSaslStanza::BuildPlainSaslXml(const std::string& username, const std::string& password)
{
	// <auth xmlns="urn:ietf:params:xml:ns:xmpp-sasl" mechanism="PLAIN">
	//		NullUsernameNullPassword
	// </auth>
	TiXmlElement node(NODE_AUTH);
	node.SetAttribute(NS_XML, NS_VAL_SASL);
	node.SetAttribute(ATTRIBUTE_AUTH_MECHANISM, ATTRIBUTE_SASL_PLAIN);


	// build the auth string which is the username and password preceded by NULL and the lot Base64 encoded
	cByteStream saslByteStream(1024);
	u8 nullByte = NULL;
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (&nullByte), 1);
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (username.c_str()), static_cast<u32>(username.size()));
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (&nullByte), 1);
	saslByteStream.StreamBytes(reinterpret_cast<const u8*> (password.c_str()), static_cast<u32>(password.size()));

	// Base64 encode the auth details
	char b64Auth[1024];
	memset(b64Auth, 0, sizeof(b64Auth));
	s32 b64AuthSize = EncodeBase64(reinterpret_cast<const u8*> (saslByteStream.Data()), saslByteStream.Size(), reinterpret_cast<u8*> (b64Auth), sizeof(b64Auth));
	assert(b64AuthSize < sizeof(b64Auth));


	TiXmlText authXmlText(b64Auth);
	node.InsertEndChild(authXmlText);

	mXmlDoc.InsertEndChild(node);
}// END BuildPlainSaslXml



bool cSaslStanza::ParseResponse(const std::string& strXml)
{
	// <success xmlns='urn:ietf:params:xml:ns:xmpp-sasl'/>
	// <failure xmlns='urn:ietf:params:xml:ns:xmpp-sasl'><not-authorized/></failure>

	TiXmlDocument xmlDoc;
	xmlDoc.Parse(strXml.c_str());
	bool hasError = xmlDoc.Error();
	if(hasError)
	{
		return false;
	}

	// simply verify the node namespace and its value
	TiXmlElement* pResultNode = xmlDoc.RootElement();
	if( pResultNode == NULL)
	{
		assert(0);
		return false;
	}

	const char* szXmlNs = pResultNode->Attribute(NS_XML);
	if(szXmlNs == NULL ||
	   szXmlNs != std::string(NS_VAL_SASL))
	{
		return false;
	}

	if(std::string(pResultNode->Value()) != NODE_SUCCESS)
	{
		assert(0);
		return false;
	}
	return true;
}// END ParseResponse



} // namespace net

