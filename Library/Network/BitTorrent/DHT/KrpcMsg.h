// Jon Bellamy 19/11/2009


#ifndef KRPC__H_
#define KRPC__H_


#if USE_PCH
#include "stdafx.h"
#endif

#include <string>
#include <assert.h>

#include "Network/SockAddr.h"
#include "Network/BitTorrent/dht/Node.h"
#include "Network/BitTorrent/BEncoding/BencodedDictionary.h"
#include "Network/BitTorrent/BEncoding/BencodedString.h"
#include "Network/BitTorrent/BEncoding/BencodedInt.h"
#include "Network/BitTorrent/BEncoding/BencodedList.h"


class cDhtTask;


//////////////////////////////////////////////////////////////////////////
// Base

class cKrpcMsg
{
public:
	cKrpcMsg() : mpTask(NULL), mSendTime(0) {}
	virtual ~cKrpcMsg();
	cKrpcMsg(const char* messageType, u16 responseTransactionId);

	// mainly for debugging
	void Load(const char* data);
	void Save(const char* fn) const;
	virtual std::string MsgTypeAsString() const { assert(0); return "[badlog]"; }

	virtual bool Parse(const char * data, u32 dataSize);

	const std::string& Message() const { return mKrpcDict.RawData(); }

	u16 TransactionId() const;

	const cBencodedString* MessageType() const;

	void SetTask(cDhtTask* pTask) { mpTask = pTask; }
	cDhtTask* GetTask() const { return mpTask; }

	void SendTime(u32 time) { mSendTime = time; }
	u32 SendTime() const { return mSendTime; }

	void SentTo(const cDhtNode& to) { mSentTo = to; }
	const cDhtNode& SentTo() const { return mSentTo; }


private:
	cKrpcMsg(const cKrpcMsg& rhs);
	const cKrpcMsg& operator= (const cKrpcMsg& rhs);
	bool operator== (const cKrpcMsg& rhs);

protected:
	// this will hold what we send
	cBencodedDictionary mKrpcDict;

	// the task that created us
	mutable cDhtTask* mpTask;
	u32 mSendTime;
	cDhtNode mSentTo;
};


//////////////////////////////////////////////////////////////////////////
// Message Types


// if the message type is a query then the message will have a method name (benString) and the arguments (benDict)
class cKrpcQuery : public cKrpcMsg
{
public:
	cKrpcQuery() {}
	cKrpcQuery(const char* methodName);

	std::string MsgTypeAsString() const { return MethodName()->Get(); }

	virtual bool Parse(const char* data, u32 dataSize);

	const cBencodedString* MethodName() const;
	
	void AddArgument(const char* argName, const cBencodedType& argValue);
	const cBencodedType* GetArgumentValue(const char* argName) const;


protected:
	cBencodedDictionary mArgumentsDict;
};



class cKrpcResponse : public cKrpcMsg
{
public:
	cKrpcResponse() {}
	cKrpcResponse(u16 responseTransactionId);
	
	std::string MsgTypeAsString() const { return "response"; }

	virtual bool Parse(const char* data, u32 dataSize);

	void AddElement(cBencodedDictionary::DictPair elem) { mResponseDict.AddElement(elem); }
	const cBencodedType* GetResponseValue(const char* argName) const;
	
	// called before sending and after adding all the return values
	void Write();

private:
	cBencodedDictionary mResponseDict;
};



class cKrpcError : public cKrpcMsg
{
public:
	cKrpcError();
	cKrpcError(u16 responseTransactionId, u32 errCode, const char* errMsg);

	std::string MsgTypeAsString() const { return "error"; }
	
	virtual bool Parse(const char* data, u32 dataSize);

	u32 ErrorCode() const { return mErrorCode; }
	const std::string& ErrorMessage() const  { return mStrError; }

private:

	// For sending
	cBencodedList mList;

	u32 mErrorCode;
	std::string mStrError;
};



//////////////////////////////////////////////////////////////////////////
// KRPC Query Types

class cKrpcPingQuery : public cKrpcQuery
{
public:
	cKrpcPingQuery(const u8* localNodeId);
	//std::string MsgTypeAsString() const { return "Ping"; }
};



class cKrpcFindNodeQuery : public cKrpcQuery
{
public:
	cKrpcFindNodeQuery(const u8* localNodeId, const u8* requestedNodeId);
	//std::string MsgTypeAsString() const { return "FindNode"; }
};



class cKrpcGetPeersQuery : public cKrpcQuery
{
public:
	cKrpcGetPeersQuery(const u8* localNodeId, const u8* infoHash);
	//std::string MsgTypeAsString() const { return "GetPeers"; }
};



class cKrpcAnnouncePeerQuery : public cKrpcQuery
{
public:
	cKrpcAnnouncePeerQuery(const u8* localNodeId, const u8* infoHash, u16 port, const std::string& token);
	//std::string MsgTypeAsString() const { return "Announce-Peer"; }
};




extern void OnReceiveKrpcDatagram(const net::cSockAddr& from, const char* data, u32 dataSize);



//////////////////////////////////////////////////////////////////////////
// incoming KRPC error

extern void OnReceiveKrpcError(const net::cSockAddr& from, const cKrpcError& msg);



//////////////////////////////////////////////////////////////////////////
// incoming KRPC query

extern void OnReceiveKrpcQuery(const net::cSockAddr& from, const cKrpcQuery& query);

extern void OnReceiveKrpcPingMsg(const net::cSockAddr& from, const cKrpcQuery& query);
extern void OnReceiveKrpcFindNodeMsg(const net::cSockAddr& from, const cKrpcQuery& query);
extern void OnReceiveKrpcGetPeersMsg(const net::cSockAddr& from, const cKrpcQuery& query);
extern void OnReceiveKrpcAnnounceMsg(const net::cSockAddr& from, const cKrpcQuery& query);




//////////////////////////////////////////////////////////////////////////
// incoming KRPC responses

extern void OnReceiveKrpcResponse(const net::cSockAddr& from, const cKrpcResponse& response);

// sent is what we sent that the passed response is referring to
extern void OnReceiveKrpcPingResponse(const net::cSockAddr& from, const cKrpcQuery& sent, const cKrpcResponse& response);
extern void OnReceiveKrpcFindNodeResponse(const net::cSockAddr& from, const cKrpcQuery& sent, const cKrpcResponse& response);
extern void OnReceiveKrpcGetPeersResponse(const net::cSockAddr& from, const cKrpcQuery& sent, const cKrpcResponse& response);
extern void OnReceiveKrpcAnnounceResponse(const net::cSockAddr& from, const cKrpcQuery& sent, const cKrpcResponse& response);



#endif // KRPC__H_