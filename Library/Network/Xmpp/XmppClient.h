// Jon Bellamy 
// Xmpp (Extensible Messaging & Presence Protocol) is described in RFC's 3920 & 3921
// http://xmpp.org/rfcs/rfc3920.html
// http://xmpp.org/rfcs/rfc3921.html


#ifndef XMPP_CLIENT__H
#define XMPP_CLIENT__H

#include <string>
#include <vector>

#include "Network/SslSocket.h"
#include "Network/bytestream.h"
#include <Network/Xmpp/XmppValues.h>
#include "Network/Xmpp/Stanza.h"
#include "Network/Xmpp/Roster.h"
#include <Network/Xmpp/StreamFeatures.h>
#include "General/Timer.h"


namespace net {



class cXmppClient
{
public:
	
	cXmppClient();
	virtual ~cXmppClient();

private:
	cXmppClient (const cXmppClient& rhs);
	const cXmppClient& operator= (const cXmppClient& rhs);
	bool operator== (const cXmppClient& rhs) const;


public:

	void Process();

	bool Connect(const std::string& serverUrl, const std::string& username, const std::string& password, const std::string& resource);
	void CloseConnection();


	typedef enum
	{
		OFFLINE=0,
		CONNECTING,
		NEW_STREAM,
		STREAM_FEATURES,
		SECURING_SESSION,
		AUTHENTICATING,
		ESTABLISHING_SESSION,
		CONNECTED,

		INVALID_STATE
	}eState;

	eState State() const { return mState; }
	

private:

	enum
	{
		RECV_BUFFER_SIZE = 32 * 1024,
		DEFAULT_WAIT_TIMEOUT = 1000 * 30,
		TIMEOUT_INACTIVE = 0xFFFFFFFF
	};

	typedef bool (*StanaReplyRecvdCb) (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam);

	typedef struct  
	{
		cXmppStanza* mpStanza;
		StanaReplyRecvdCb mReplyCb;
		void* mReplyParam;
	}SentStanzaRecord;


	bool SendStanza(const cXmppStanza& stanza, StanaReplyRecvdCb replyCb=NULL, void* replyParam=NULL);
	void ProcessIncomingStanzas();

	const SentStanzaRecord* GetSentStanzaRecord(u32 id);
	void RemoveStoredStanza(u32 id);
	void RemoveAllStoredStanzas();
	
	bool XmlDocRecvd(u32* pEndByteIndexOut);
	bool RecvXmlStanza(std::string* xmlOut);
	bool RecvOpenStream();



	//////////////////////////////////////////////////////////////////////////
	// Roster, Presence and IM Message sending

	void SendGetRoster();
	void SendPresenceStatus(XmppPresenceShowStatus status, const std::string& statusStr);
	void SendChatMessage(const cJid& to, const std::string& msgBody, XmppChatState chatState);



	//////////////////////////////////////////////////////////////////////////
	// Stanza reply callbacks
	
	static bool PresenceUpdateReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam);
	static bool BindResourceReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam);
	static bool EstablishSessionReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam);



	//////////////////////////////////////////////////////////////////////////
	// Client States

	void ProcessState_Connecting();
	void ProcessState_NewStream();
	void ProcessState_StreamFeatures();
	void ProcessState_SecuringSession();
	void ProcessState_Authenticating();
	void ProcessState_EstablishingSession();
	void ProcessState_Connected();


	//////////////////////////////////////////////////////////////////////////
	// Inline accessors

	void State(eState state);
	u32 NumberOfStanzasAwaitingReply() const;



	//////////////////////////////////////////////////////////////////////////
	// Member Data


	// all sessions are capable of being secured so lets just stick with an ssl socket
	cSslSocket mSocket;
	cByteStream mRecvBuffer;

	XmppClientError mClientError;

	std::string mServerUrl;
	std::string mUsername;
	std::string mPassword;
	std::string mResource;

	cJid mFullJid;
	cXmppRoster mFriendRoster;

	cXmppStreamFeatures mServerStreamFeatures;

	typedef std::vector<SentStanzaRecord> StanzaVector;
	typedef StanzaVector::iterator StanzaVectorIterator;
	typedef StanzaVector::const_iterator StanzaVectorConstIterator;
	StanzaVector mStanzasAwaitingReply;

	bool mWaitingForResponse;

	cTimer mTimer;
	u32 mTimeoutStartTime;

	eState mState;

	// Every time we start a new stream we go through the NEW_STREAM & STREAM_FEATURES states. After the STREAM_FEATURES
	// state we will transition to the following state.
	eState mNewStreamState;
};



inline void cXmppClient::State(eState state) 
{ 
	mTimeoutStartTime = 0;
	mState = state; 
}// END State



inline u32 cXmppClient::NumberOfStanzasAwaitingReply() const 
{ 
	return static_cast<u32> (mStanzasAwaitingReply.size()); 
}// END NumberOfStanzasAwaitingReply




} // namespace net


#endif // XMPP_CLIENT__H