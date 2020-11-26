// Jon Bellamy 18/11/2009


#ifndef DHT_NODE_H_
#define DHT_NODE_H_

#if USE_PCH
#include "stdafx.h"
#endif

#include <string>

#include "Network/BitTorrent/dht/NodeId.h"
#include "Network/SockAddr.h"


class cDhtNode
{
public:
	cDhtNode();	
	cDhtNode(const net::cSockAddr& addr, const std::string& nodeId);	
	cDhtNode(const net::cSockAddr& addr, const cDhtNodeId& nodeId);	
	cDhtNode(const net::cSockAddr& addr);
	cDhtNode(const cDhtNode& rhs);

	const cDhtNode& operator= (const cDhtNode& rhs);
	bool operator== (const cDhtNode& rhs) const;
	bool operator< (const cDhtNode& rhs) const { return Id() < rhs.Id(); }

	const cDhtNodeId& Id() const { return mNodeId; }

	const net::cSockAddr& Address() const { return mAddr; }

	std::string ToCompactRepresentation() const;
	void FromCompactRepresentation(const u8* compactData);

	enum
	{
		INVALID_NODE_DISTANCE = 0xFFFFFFFF,

		COMPACT_NODE_SIZE = cDhtNodeId::NODE_ID_SIZE + 6			// 20 byte id + 4 byte ip + 2 byte port
	};


private:

	net::cSockAddr mAddr;

	cDhtNodeId mNodeId;

	// TODO : almost certainly an array of objects. The class needs to contain:
	// info hash
	// token
	// ip & port
};




#endif // DHT_NODE_H_