// Jon Bellamy 
// Xmpp (Extensible Messaging & Presence Protocol) is described in RFC's 3920 & 3921
// http://xmpp.org/rfcs/rfc3920.html
// http://xmpp.org/rfcs/rfc3921.html


#include "XmppClient.h"

#include <assert.h>
#include <memory>

#include "Network/Dns.h"
#include "Network/IpAddr.h"
#include "Network/SockAddr.h"
#include "Network/Ports.h"


#include "3rdParty/TinyXml/tinyxml.h"
#include "File/file.h"


#define TAB 9
#define SPACE_BAR 32
bool IsCharWhiteSpace(char c)
{
	return (c == TAB || c == SPACE_BAR || c == '\n' || c == '\r');
}


namespace net {


cXmppClient::cXmppClient()
: mSocket()
, mTimer(cTimer::STOPWATCH)
, mClientError(ERROR_NONE)
, mRecvBuffer(RECV_BUFFER_SIZE)
, mState(OFFLINE)
, mNewStreamState(INVALID_STATE)
, mWaitingForResponse(false)
, mTimeoutStartTime(TIMEOUT_INACTIVE)
{
//	cFile f(cFile::READ, "c:\\test.xml", true);
//	mRecvBuffer.StreamBytes(f.CachedFileData(), f.Size());
//	ProcessState_StreamFeatures();

	mTimer.Start();
}// END cXmppClient



cXmppClient::~cXmppClient() 
{
	CloseConnection();
}// END ~cXmppClient



bool cXmppClient::Connect(const std::string& serverUrl, const std::string& username, const std::string& password, const std::string& resource)
{
	assert(State() == OFFLINE);

	bool bRet;
	cIpAddr ip;
	cDns::IpAddressFromDomainName(serverUrl.c_str(), 0, &ip);
	cSockAddr sockAddr(ip, XMPP_PORT);
	
	bRet = mSocket.OpenAndConnectUnsecured(sockAddr, false);
	if(bRet==false)
	{
		return false;
	}

	mServerUrl = serverUrl;
	mUsername = username;
	mPassword = password;
	mResource = resource;

	mTimeoutStartTime = mTimer.ElapsedMs();
	State(CONNECTING);
	return true;
}// END Connect



void cXmppClient::CloseConnection()
{
	mTimeoutStartTime = TIMEOUT_INACTIVE;

	if(mSocket.ConnectionEstablished())
	{
		std::string closeXml = cStreamStanza::CloseStreamXml();
		mSocket.Send(closeXml.c_str(), static_cast<u32>(closeXml.size()));
	}
	mSocket.Close();
	mServerStreamFeatures.Clear();

	RemoveAllStoredStanzas();
	mRecvBuffer.Clear();

	State(OFFLINE);
}// END CloseConnection



void cXmppClient::Process()
{
	mTimer.Process();

	if(mTimeoutStartTime != TIMEOUT_INACTIVE)
	{
		if(mTimer.ElapsedMs() - mTimeoutStartTime >= DEFAULT_WAIT_TIMEOUT)
		{
			printf("Xmpp client timeout, closing connection.\n");
			mClientError = ERROR_CONNECTION_TIMED_OUT;
			CloseConnection();
			return;
		}
	}

	if(State() != OFFLINE && 
	   State() != CONNECTING && 
	   mSocket.ConnectionEstablished() == false)
	{
		assert(0);
		mClientError = ERROR_TRANSPORT;
		CloseConnection();
		return;
	}

	switch(State())
	{
	case OFFLINE:
		break;

	case CONNECTING:
		ProcessState_Connecting();	
		break;

	case NEW_STREAM:
		ProcessState_NewStream();	
		break;

	case STREAM_FEATURES:
		ProcessState_StreamFeatures();	
		break;

	case SECURING_SESSION:
		ProcessState_SecuringSession();
		break;

	case AUTHENTICATING:
		ProcessState_Authenticating();
		break;

	case ESTABLISHING_SESSION:
		ProcessState_EstablishingSession();
		break;

	case CONNECTED:
		ProcessState_Connected();
		break;

	default:
		assert(0);
	}


	if(State() == ESTABLISHING_SESSION || State() == CONNECTED)
	{
		ProcessIncomingStanzas();
	}
}// END Process



bool cXmppClient::SendStanza(const cXmppStanza& stanza, StanaReplyRecvdCb replyCb, void* replyParam)
{
	// debug - presence stanza's don't have an id
	if(typeid(stanza) == typeid(cPresenceStanza))
	{
		assert(stanza.HasId()==false);
	}
	
	std::string stanzaXml = stanza.AsString();
	s32 bytesSent = mSocket.Send(stanzaXml.c_str(), static_cast<u32>(stanzaXml.size()));

	if(stanza.HasId())
	{
		assert(replyCb);

		cXmppStanza* pStanzaCopy = stanza.Clone();		
		// if this assert fires then we have a non standard stanza (not msg, iq or presence) with an Id
		assert(pStanzaCopy);		
		if(pStanzaCopy)
		{
			SentStanzaRecord record;
			memset(&record, 0, sizeof(SentStanzaRecord));
			record.mpStanza = pStanzaCopy;
			record.mReplyCb = replyCb;
			record.mReplyParam = replyParam;
			mStanzasAwaitingReply.push_back(record);
		}
	}
	else
	{
		// no id but reply cb??
		assert(replyCb==NULL);
	}

	return bytesSent == stanzaXml.size();
}// END SendStanza



void cXmppClient::ProcessIncomingStanzas()
{
	std::string xml;
	if(RecvXmlStanza(&xml))
	{
		TiXmlElement streamElement("");
		streamElement.Parse(xml.c_str(), NULL, TIXML_DEFAULT_ENCODING);
		if(streamElement.Value()==NULL)
		{
			assert(0);
			CloseConnection();
		}
		std::string stanzaType(streamElement.Value());
		
		// multiple code paths out of this function and we want this deleted
		std::auto_ptr<cXmppStanza> pReplyStanza;
	
		
		// hmm what can a reply be? Not a message or presence. Does everything with an get a reply? Just IQ -> Result msgs coming back?
		// does a client ever get incoming queries?
		if(stanzaType == STANZA_MESSAGE)
		{
			assert(0);
			pReplyStanza.reset(new cMessageStanza());
		}
		else if(stanzaType == STANZA_PRESENCE)
		{
			pReplyStanza.reset(new cPresenceStanza());
		}
		else if(stanzaType == STANZA_IQ)
		{
			pReplyStanza.reset(new cIqStanza());
		}
		else
		{
			assert(0);
			CloseConnection();
			return;
		}
		assert(pReplyStanza.get());
		pReplyStanza->Parse(xml);


		if(typeid(*pReplyStanza) == typeid(cIqStanza))
		{
			cIqStanza* pIqStanza = static_cast<cIqStanza*> (pReplyStanza.get());
			if(pIqStanza->IsResult() == false)
			{
				assert(0);
				CloseConnection();
				return;
			}

			// get the stored stanza that this is the reply to
			const SentStanzaRecord* pStanzaRecord = GetSentStanzaRecord(pReplyStanza->Id());
			if(pStanzaRecord == NULL || pStanzaRecord->mReplyCb == NULL)
			{
				assert(0);
				CloseConnection();
				return;
			}

			// fire the callback
			if(pStanzaRecord->mReplyCb(*(pStanzaRecord->mpStanza), *pReplyStanza, pStanzaRecord->mReplyParam) == false)
			{
				assert(0);
				CloseConnection();
				return;
			}

			RemoveStoredStanza(pReplyStanza->Id());
		}
		else if(typeid(*pReplyStanza) == typeid(cPresenceStanza))
		{
			// presence stanza's don't have an id
			cPresenceStanza* pPresenceStanza = static_cast<cPresenceStanza*> (pReplyStanza.get());
			mFriendRoster.PresenceUpdate(*pPresenceStanza);
		}
		else
		{
			assert(0);
		}


	}
}// END ProcessIncomingStanzas



const cXmppClient::SentStanzaRecord* cXmppClient::GetSentStanzaRecord(u32 id)
{
	for(StanzaVectorIterator iter = mStanzasAwaitingReply.begin(); iter != mStanzasAwaitingReply.end(); iter++)
	{
		const SentStanzaRecord& stanzaRecord = *iter;
		assert(stanzaRecord.mpStanza);
		if(stanzaRecord.mpStanza->Id() == id)
		{
			return &stanzaRecord;
		}
	}
	return NULL;
}// END GetSentStanzaRecord



void cXmppClient::RemoveStoredStanza(u32 id)
{
	for(StanzaVectorIterator iter = mStanzasAwaitingReply.begin(); iter != mStanzasAwaitingReply.end(); iter++)
	{
		const SentStanzaRecord& stanzaRecord = *iter;
		assert(stanzaRecord.mpStanza);
		if(stanzaRecord.mpStanza->Id() == id)
		{
			mStanzasAwaitingReply.erase(iter);
			return;
		}
	}
}// END RemoveStoredStanza



void cXmppClient::RemoveAllStoredStanzas()
{
	while(mStanzasAwaitingReply.size() > 0)
	{
		SentStanzaRecord& stanzaRecord = mStanzasAwaitingReply.back();
		assert(stanzaRecord.mpStanza);
		delete stanzaRecord.mpStanza;
		mStanzasAwaitingReply.pop_back();
	}
}// END RemoveAllStoredStanzas



// An xml doc is simply some xml whose depth is balanced, eg - <node>hello world</node> - is a valid xml doc
bool cXmppClient::XmlDocRecvd(u32* pEndByteIndexOut)
{
/*
	No need to worry about embedded control character:
	In text and attribute values, you need to escape ASCII characters like the angle brackets ($<$$>$). Most common encodings are:
	"   &quot;
	<   &lt;
	>   &gt;
	&   &amp;
*/

	bool firstNonWhitespaceRead=false;
	s32 depth=0;
	for(u32 i=0; i < mRecvBuffer.Size(); i++)
	{
		// ignore whitespace
		if(IsCharWhiteSpace(mRecvBuffer[i]))
		{
			continue;
		}

		// check the first char is an open tag <
		if(firstNonWhitespaceRead == false)
		{
			firstNonWhitespaceRead=true;					
			if(mRecvBuffer[i] != '<')
			{
				assert(0);

				// stream is knackered
				CloseConnection();
				return false;
			}
		}

		// new open tag increases depth
		if(mRecvBuffer[i] == '<' && mRecvBuffer[i+1] != '/')
		{
			depth++;
		}

		// close tag method A <node type="1" />
		if(mRecvBuffer[i] == '/' && mRecvBuffer[i+1] == '>')
		{
			depth--;

			if(depth == 0)
			{
				// complete doc
				*pEndByteIndexOut = i+2;
				return true;
			}
		}

		// close tag method B <node>foo</node>
		if(mRecvBuffer[i] == '<' && mRecvBuffer[i+1] == '/')
		{
			depth--;

			if(depth == 0)
			{
				// complete doc is at the end of this tag
				u32 j=i+2;
				while(mRecvBuffer[j] != '>')
				{
					j++;

					// somethings very wrong
					assert(j-i < 64);
				}
				*pEndByteIndexOut = j+1;
				return true;
			}
		}
	}
	return false;
}// END XmlDocRecvd



bool cXmppClient::RecvXmlStanza(std::string* xmlOut)
{
	if(xmlOut==NULL)
	{
		assert(0);
		return false;
	}

	s32 bytesRecvd = mSocket.Recv(&mRecvBuffer, mRecvBuffer.Capacity(), false);	
	if(bytesRecvd < 0 || mRecvBuffer.Size() == 0)
	{
		return false;
	}

	u32 index;
	bool gotXmlDoc = XmlDocRecvd(&index);
	if(gotXmlDoc)
	{
		xmlOut->insert(0, reinterpret_cast<const char*>(mRecvBuffer.Data()), index);	
	
#if VALIDATE_INCOMING_STANZAS_ON_RECV
		// doc needs a NULL terminated string
		TiXmlDocument doc;
		doc.Parse(strXml.c_str());
		bool isError = doc.Error();
		assert(isError==false);

		if(isError == false)
		{
			mRecvBuffer.RemoveBytes(0, index);
			
			// only for debug
			mRecvBuffer.ZeroUnused();
		}
		return isError;
#else
		mRecvBuffer.RemoveBytes(0, index);
		mRecvBuffer.ZeroUnused();
		return true;
#endif	
	}

	return false;
}// END RecvXmlStanza



// special case as the stream open is not actually a valid xml doc, it is a single open node
bool cXmppClient::RecvOpenStream()
{
	s32 bytesRecvd = mSocket.Recv(&mRecvBuffer, mRecvBuffer.Capacity(), false);	
	if(bytesRecvd <= 0)
	{
		return false;
	}
	assert(mRecvBuffer.Size() > 0);

	// we must be at the beginning of valid xml
	if(mRecvBuffer[0] != '<')
	{
		assert(0);
		CloseConnection();
		return false;
	}

/* example response 
	<?xml version='1.0'?>
	<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' id='3824579465' from='jabberd.eu' version='1.0' xml:lang='en'>
		<stream:features>
			<starttls xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>
			<mechanisms xmlns='urn:ietf:params:xml:ns:xmpp-sasl'>
				<mechanism>DIGEST-MD5</mechanism>
				<mechanism>PLAIN</mechanism>
			</mechanisms>
			<register xmlns='http://jabber.org/features/iq-register'/>
		</stream:features>
*/

	bool hasXmlDeclaration = mRecvBuffer[0] == '<' && mRecvBuffer[1] == '?';
	u32 numClosingTagsRequired = hasXmlDeclaration ? 2 : 1;
	u32 numClosingTagsFound=0;

	u32 featuresStartIndex=0;
	
	for(u32 i=0; i < mRecvBuffer.Size(); i++)
	{		
		featuresStartIndex++;

		// ignore whitespace
		if(IsCharWhiteSpace(mRecvBuffer[i]))
		{
			continue;
		}

		if(mRecvBuffer[i] == '>')
		{
			numClosingTagsFound++;
			if(numClosingTagsFound == numClosingTagsRequired)
			{
				break;
			}
		}
	}

	// have we completely received the open stream node
	if(numClosingTagsFound != numClosingTagsRequired)
	{
		return false;
	}

	// parse the stream node
	cStreamStanza streamStanza;
	bool ret = streamStanza.Parse(mRecvBuffer.AsString());
	if(!ret)
	{
		// If the stream node is invalid then we cannot go on.
		assert(0);
		CloseConnection();
		return false;
	}


	mRecvBuffer.RemoveBytes(0, featuresStartIndex);
	mRecvBuffer.ZeroUnused();

	return true;
}// END RecvOpenStream



//////////////////////////////////////////////////////////////////////////
// Roster and Message sending


void cXmppClient::SendGetRoster()
{
	if(State() != CONNECTED)
	{
		assert(0);
		return;
	}
	cIqStanza getRosterStanza;
	getRosterStanza.GetRoster();
	SendStanza(getRosterStanza, cXmppRoster::RosterUpdateReplyCb, &mFriendRoster);
}// END SendGetRoster



void cXmppClient::SendPresenceStatus(XmppPresenceShowStatus onlineStatus, const std::string& statusStr)
{
	if(State() != CONNECTED)
	{
		assert(0);
		return;
	}
	cPresenceStanza stanza(&mFullJid, NULL);
	stanza.AddShowNode(onlineStatus);
	if(statusStr.size() > 0)
	{
		stanza.AddStatusNode(statusStr);
	}
	SendStanza(stanza, NULL, NULL); //PresenceUpdateReplyCb, this);
}// END SendPresenceStatus



void SendChatMessage(const cJid& to, const std::string& msgBody, XmppChatState chatState)
{
	// TODO 
	assert(0);
}// END SendChatMessage



//////////////////////////////////////////////////////////////////////////
// Stanza reply callbacks


bool cXmppClient::PresenceUpdateReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam)
{
	cXmppClient* pThis = reinterpret_cast<cXmppClient*> (pParam);
	assert(pThis);
	const cIqStanza& iqReply = static_cast<const cIqStanza&> (reply);
	printf("PresenceUpdateReplyCb\n");
	return true;
}// END PresenceUpdateReplyCb



bool cXmppClient::BindResourceReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam)
{
	cXmppClient* pThis = reinterpret_cast<cXmppClient*> (pParam);
	assert(pThis);
	const cIqStanza& iqReply = static_cast<const cIqStanza&> (reply);

	printf("BindResourceReplyCb\n");

	/*
	<iq id='FF000000' type='result'>
		<bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'>
			<jid>jonb_rs2@jabberd.eu/Test Resource</jid>
		</bind>
	</iq>
	*/

	// check we have a bind node and that the namespace is correct
	const TiXmlElement* pBindNode = iqReply.IqRootNode()->FirstChildElement(NODE_BIND);
	if(pBindNode == NULL)
	{
		return false;
	}
	const char* szXmlNs = pBindNode->Attribute(NS_XML);
	if(szXmlNs == NULL ||
		szXmlNs != std::string(NS_VAL_BIND))
	{
		return false;
	}

	// get the jid node, the jid nodes child is the text element
	const TiXmlElement* pJidNode = pBindNode->FirstChildElement(IQ_STANZA_JID);
	if(pJidNode == NULL || pJidNode->FirstChild() == NULL)
	{
		return false;
	}
	std::string jid = pJidNode->FirstChild()->Value();

	// store & validate jid
	pThis->mFullJid = cJid(jid.c_str());
	if(pThis->mFullJid.IsFullJid() == false)
	{
		return false;
	}

	return true;
}// END BindResourceReplyCb



bool cXmppClient::EstablishSessionReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam)
{
	cXmppClient* pThis = reinterpret_cast<cXmppClient*> (pParam);
	assert(pThis);
	const cIqStanza& iqReply = static_cast<const cIqStanza&> (reply);

	printf("EstablishSessionReplyCb\n");

	/*
	<iq type='result' id='4278190081'>
		<session xmlns='urn:ietf:params:xml:ns:xmpp-session'/>
	</iq>
	*/

	// simply verify the session node along with its namespace
	const TiXmlElement* pSessionNode = iqReply.IqRootNode()->FirstChildElement(NODE_SESSION);
	if(pSessionNode == NULL)
	{
		return false;
	}
	const char* szXmlNs = pSessionNode->Attribute(NS_XML);
	if(szXmlNs == NULL ||
		szXmlNs != std::string(NS_VAL_SESSION))
	{
		return false;
	}

	return true;
}// END EstablishSessionReplyCb




//////////////////////////////////////////////////////////////////////////
// Client States


void cXmppClient::ProcessState_Connecting()
{
	if(mSocket.ConnectionEstablished())
	{
		mNewStreamState = SECURING_SESSION;
		State(NEW_STREAM);
	}
}// END ProcessState_Connecting



void cXmppClient::ProcessState_NewStream()
{
	if(!mWaitingForResponse)
	{
		// send open stream
		cStreamStanza streamStanza(mServerUrl.c_str());
		if(SendStanza(streamStanza) == false)
		{
			assert(0);
			mClientError = ERROR_TRANSPORT;		
			CloseConnection();
			return;
		}	
		mWaitingForResponse = true;
		mTimeoutStartTime = mTimer.ElapsedMs();
	}
	else
	{
		if(RecvOpenStream())
		{
			// we have their open stream
			mWaitingForResponse = false;
			State(STREAM_FEATURES);
		}
	}
}// END ProcessState_NewStream



void cXmppClient::ProcessState_StreamFeatures()
{
	std::string xml;
	if(RecvXmlStanza(&xml))
	{
		cStreamFeaturesStanza featuresStanza;
		featuresStanza.Parse(xml);
		mServerStreamFeatures.Clear();
		mServerStreamFeatures.PopulateFromStanza(featuresStanza);

		State(mNewStreamState);
		mNewStreamState = INVALID_STATE;
	}
}// END ProcessState_StreamFeatures



void cXmppClient::ProcessState_SecuringSession()
{
	if(!mWaitingForResponse)
	{
		if(mServerStreamFeatures.SupportsFeature(cXmppStreamFeatures::TLS))
		{
			cStartTlsStanza tlsStanza;
			if(SendStanza(tlsStanza) == false)
			{
				assert(0);
				mClientError = ERROR_TRANSPORT;				
				CloseConnection();
				return;
			}
			mTimeoutStartTime = mTimer.ElapsedMs();
			mWaitingForResponse = true;
		}
		else
		{
			assert(0);
			mClientError = ERROR_SERVER_STREAM_FEATURE_INVALID;
			CloseConnection();
			return;
		}
	}
	else
	{
		// <proceed xmlns='urn:ietf:params:xml:ns:xmpp-tls'/>
		bool bRet;
		std::string xml;
		bRet = RecvXmlStanza(&xml);
		if(bRet)
		{
			mWaitingForResponse = false;

			// TODO : This NEEDS sorting
			mSocket.SetBlockingState(true);

			// Fire up OpenSSL & secure the socket 
			bRet = mSocket.SecureConnection();
			if(bRet==false)
			{
				assert(0);
				CloseConnection();
				return;
			}

			mSocket.SetBlockingState(false);

			mNewStreamState = AUTHENTICATING;
			State(NEW_STREAM);
		}
	}
}// END ProcessState_SecuringSession



void cXmppClient::ProcessState_Authenticating()
{
	if(!mWaitingForResponse)
	{
		if(mServerStreamFeatures.SupportsFeature(cXmppStreamFeatures::SASL_PLAIN))
		{
			cSaslStanza saslStanza(cSaslStanza::PLAIN, mUsername.c_str(), mPassword.c_str());
			if(SendStanza(saslStanza) == false)
			{
				assert(0);	
				mClientError = ERROR_TRANSPORT;
				CloseConnection();
				return;
			}
			mTimeoutStartTime = mTimer.ElapsedMs();
			mWaitingForResponse = true;
		}
		else
		{
			assert(0);
			mClientError = ERROR_SERVER_STREAM_FEATURE_INVALID;
			CloseConnection();
			return;
		}
	}
	else
	{
		bool bRet;
		std::string xml;
		bRet = RecvXmlStanza(&xml);
		if(bRet)
		{
			if(cSaslStanza::ParseResponse(xml) == false)
			{
				mClientError = ERROR_AUTHENTICATION_FAILED;
				CloseConnection();
				return;
			}

			mWaitingForResponse = false;
			mNewStreamState = ESTABLISHING_SESSION;
			State(NEW_STREAM);
		}
	}
}// END ProcessState_Authenticating



void cXmppClient::ProcessState_EstablishingSession()
{
	if(!mWaitingForResponse)
	{
		// bind the resource
		cIqStanza bindResourceStanza;
		bindResourceStanza.BindResource(mResource.c_str());
		if(SendStanza(bindResourceStanza, BindResourceReplyCb, this) == false)
		{
			assert(0);
			mClientError = ERROR_TRANSPORT;		
			CloseConnection();
			return;
		}

		// establish session
		// TODO : We don't always have to do this, only if the stream options told us to.
		cIqStanza sessionStanza;
		sessionStanza.EstablishSession();
		if(SendStanza(sessionStanza, EstablishSessionReplyCb, this) == false)
		{
			assert(0);
			mClientError = ERROR_TRANSPORT;		
			CloseConnection();
			return;
		}

		mTimeoutStartTime = mTimer.ElapsedMs();
		mWaitingForResponse = true;
	}
	else
	{
		if(NumberOfStanzasAwaitingReply() == 0)
		{
			// if we get here then no errors were generated, ie we are good to go
			State(CONNECTED);
			mTimeoutStartTime = TIMEOUT_INACTIVE;

			// sync the friends list
			SendGetRoster();
			SendPresenceStatus(PRESENCE_SHOW_CHAT, "My status string");
		}
	}
}// END ProcessState_EstablishingSession



void cXmppClient::ProcessState_Connected()
{
	assert(mTimeoutStartTime == TIMEOUT_INACTIVE);

	//printf("ProcessState_Connected\n");

}// END ProcessState_Connected



} // namespace net

