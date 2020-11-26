#include "Roster.h"

#include <assert.h>

#include "TinyXml/tinyxml.h"


namespace net {


cXmppRoster::cXmppRoster()
{
}// END cXmppRoster



// called when we receive a presence stanza
bool cXmppRoster::PresenceUpdate(const cPresenceStanza& presenceStanza)
{
	cJid from = presenceStanza.From();
	if(from.IsFullJid()==false)
	{
		assert(0);
		return false;
	}

	XmppPresenceShowStatus status = presenceStanza.ShowStatus();
	
	FriendInfo* pFriendInfo = GetFriendInfo(from);
	if(pFriendInfo==NULL)
	{
		return false;
	}

	// TODO : update friend data

	return true;
}// END PresenceUpdate



bool cXmppRoster::RosterUpdateReplyCb (const cXmppStanza& sent, const cXmppStanza& reply, void* pParam)
{ 
	cXmppRoster* pThis = reinterpret_cast<cXmppRoster*> (pParam);
	assert(pThis);
	const cIqStanza& iqReply = static_cast<const cIqStanza&> (reply);

	/*
	<iq to='juliet@example.com/balcony' type='result' id='roster_1'>
	  <query xmlns='jabber:iq:roster'>
		<item jid='romeo@example.net'
			  name='Romeo'
			  subscription='both'>
		  <group>Friends</group>
		</item>
		<item jid='mercutio@example.org'
			  name='Mercutio'
			  subscription='from'>
		  <group>Friends</group>
		</item>
		<item jid='benvolio@example.org'
			  name='Benvolio'
			  subscription='both'>
		  <group>Friends</group>
		</item>
	  </query>
	</iq>
	*/

	const TiXmlElement* pQueryNode = iqReply.IqRootNode()->FirstChildElement(IQ_STANZA_QUERY);
	if(pQueryNode == NULL)
	{
		return false;
	}
	const char* szXmlNs = pQueryNode->Attribute(NS_XML);
	if(szXmlNs == NULL ||
		szXmlNs != std::string(NS_QUERY_ROSTER))
	{
		return false;
	}

	pThis->mRoster.clear();

	// process each item node
	const TiXmlElement* pItemNode = pQueryNode->FirstChildElement(IQ_STANZA_ITEM);
	while(pItemNode)
	{
		const TiXmlElement* pGroupNode = pItemNode->FirstChildElement(IQ_STANZA_GROUP);
		if(pGroupNode != NULL)
		{
			// TODO : check the standard, is this an error?
		}
		
		const char* szJid = pItemNode->Attribute(IQ_STANZA_JID);
		if(szJid == NULL)
		{
			return false;
		}

		const char* szName = pItemNode->Attribute(IQ_STANZA_NAME);
		if(szName == NULL)
		{
			return false;
		}

		const char* szSubscription = pItemNode->Attribute(IQ_STANZA_SUBSCRIPTION);
		if(szSubscription == NULL)
		{
			return false;
		}

		
		// TODO : store ALL the friend data
		
		FriendInfo info;
		info.mJid = cJid(szJid);
		if(info.mJid.IsValid() == false)
		{
			return false;
		}
		info.mName = szName;
		//info.mOnlineStatus
		
		pThis->mRoster.push_back(info);

		pItemNode = pItemNode->NextSiblingElement(IQ_STANZA_ITEM);
	}

	return true;
}// END RosterUpdateReplyCb



FriendInfo* cXmppRoster::GetFriendInfo(const cJid& jid)
{
	for(u32 i=0; i < mRoster.size(); i++)
	{
		FriendInfo& info = mRoster[i];
		if(info.mJid == jid)
		{
			return &info;
		}
	}
	return NULL;
}// END GetFriendInfo



} // namespace net

