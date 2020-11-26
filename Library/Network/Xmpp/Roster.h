#ifndef ROSTER__H
#define ROSTER__H

#include <vector>
#include <string>

#include "Jid.h"
#include "Stanza.h"

namespace net {




class FriendInfo
{
public:
	FriendInfo()
	: mJid()
	, mOnlineStatus(OFFLINE)
	, mName("")
	{
	}

	const FriendInfo& operator= (const FriendInfo& rhs)
	{
		mJid = rhs.mJid;
		mOnlineStatus = rhs.mOnlineStatus;
		mName = rhs.mName;
		return *this;
	}

	cJid mJid;
	XmppOnlineStatus mOnlineStatus;
	std::string mName;


	// TODO ...
	// subscription
	// PresenceShowStatus 
	// ChatState mChatState <<-- per conversation though so prob not here
};


class cXmppRoster
{
public:	
	cXmppRoster();
	virtual ~cXmppRoster() {}

private:
	bool operator== (const cXmppRoster& rhs) const;
	cXmppRoster (const cXmppRoster& rhs);
	const cXmppRoster& operator= (const cXmppRoster& rhs);

public:

	// called when we receive a presence stanza
	bool PresenceUpdate(const cPresenceStanza& presenceStanza);

	static bool RosterUpdateReplyCb(const cXmppStanza& sent, const cXmppStanza& reply, void* pParam);	

private:

	FriendInfo* GetFriendInfo(const cJid& jid);



	typedef std::vector<FriendInfo> FriendVector;
	typedef FriendVector::iterator FriendVectorIterator;
	typedef FriendVector::const_iterator FriendVectorConstIterator;
	FriendVector mRoster;
};




} // namespace net


#endif // ROSTER__H