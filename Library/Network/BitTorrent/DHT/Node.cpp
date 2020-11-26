// Jon Bellamy 18/11/2009


#include "Node.h"

#include <assert.h>

#include "General/Endianness.h"



using namespace net;



cDhtNode::cDhtNode()
: mNodeId(INVALID_NODE_ID)
{
}// END cDhtNode


cDhtNode::cDhtNode(const cSockAddr& addr, const std::string& nodeId)
: mAddr(addr)
, mNodeId(nodeId)
{

}// END cDhtNode



cDhtNode::cDhtNode(const net::cSockAddr& addr, const cDhtNodeId& nodeId)
: mAddr(addr)
, mNodeId(nodeId)
{
}// END cDhtNode


cDhtNode::cDhtNode(const net::cSockAddr& addr)
: mAddr(addr)
, mNodeId()
{
}// END cDhtNode



cDhtNode::cDhtNode(const cDhtNode& rhs)
{
	*this = rhs;
}// END cDhtNode



const cDhtNode& cDhtNode::operator= (const cDhtNode& rhs)
{
	mAddr = rhs.mAddr;
	mNodeId = rhs.mNodeId;
	return *this;
}// END operator=



bool cDhtNode::operator== (const cDhtNode& rhs) const
{
	return mNodeId == rhs.mNodeId;
}// END operator== 



std::string cDhtNode::ToCompactRepresentation() const
{
	std::string compact;
	u32 ip = mAddr.Ip().AsU32();
	u16 port = mAddr.Port();
	
	// err ........... !!!!!!! CHECK! !!!
	//endian_swap(ip);
	
	endian_swap(port);
	compact.insert(0, reinterpret_cast<const char*>(&(Id().mNodeId[0])), cDhtNodeId::NODE_ID_SIZE);
	compact.insert(cDhtNodeId::NODE_ID_SIZE, reinterpret_cast<const char*>(&ip), sizeof(u32));
	compact.insert(cDhtNodeId::NODE_ID_SIZE + sizeof(u32), reinterpret_cast<const char*>(&port), sizeof(u16));
	return compact;
}// END ToCompactRepresentation



void cDhtNode::FromCompactRepresentation(const u8* compactData)
{
	mNodeId.FromData(compactData);

	compactData += cDhtNodeId::NODE_ID_SIZE;

	u16 port;
	memcpy(&port, &compactData[4], sizeof(port));
	endian_swap(port);

	cSockAddr addr(cIpAddr(compactData[0], compactData[1], compactData[2], compactData[3]), port);
	
	mAddr = addr;
}// END FromCompactRepresentation