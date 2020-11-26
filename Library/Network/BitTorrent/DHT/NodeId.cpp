// Jon Bellamy 18/11/2009


#include "NodeId.h"

#include <assert.h>
#include <memory.h>
#include "General/Rand.h"


cDhtNodeId::cDhtNodeId()
{
	Randomize();
}// END cDhtNodeId



cDhtNodeId::cDhtNodeId(const std::string& data)
{
	assert(data.size() == NODE_ID_SIZE);
	if(data.size() == NODE_ID_SIZE)
	{
		FromData(reinterpret_cast<const u8*>(data.c_str()));
	}
}// END cDhtNodeId



cDhtNodeId::cDhtNodeId(const cDhtNodeId& rhs)
{
	*this = rhs;
}// END cDhtNodeId



void cDhtNodeId::Randomize()
{
	for(u32 i=0; i < NODE_ID_SIZE; i++)
	{
		mNodeId[i] = (u8)Rand16(0xFF);
	}
}// END Randomize



void cDhtNodeId::FromData(const u8* data)
{
	memcpy(mNodeId, data, NODE_ID_SIZE);
}// END FromData



const cDhtNodeId& cDhtNodeId::operator= (const cDhtNodeId& rhs)
{
	memcpy(mNodeId, rhs.mNodeId, NODE_ID_SIZE);
	return *this;
}// END operator=



bool cDhtNodeId::operator== (const cDhtNodeId& rhs) const
{
	return memcmp(mNodeId, rhs.mNodeId, NODE_ID_SIZE) == 0;
}// END operator==



bool cDhtNodeId::operator< (const cDhtNodeId& rhs) const
{
	if (*this == rhs)
	{
		return false;
	}
	else
	{
		for(u32 i=0; i < NODE_ID_SIZE; i++)
		{
			if(mNodeId[i] != rhs.mNodeId[i])
			{
				if(mNodeId[i] < rhs.mNodeId[i])
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
	}
	assert(0);
	return true;
}// END operator<



bool cDhtNodeId::IsValid() const
{
	return (memcmp(mNodeId, INVALID_NODE_ID, sizeof(mNodeId)) != 0);
}// END IsValid



std::string cDhtNodeId::AsString() const
{
	return std::string(reinterpret_cast<const char*>(&mNodeId[0]), NODE_ID_SIZE);
}// END AsString


void cDhtNodeId::DebugPrint() const
{
	for(u32 i=0; i < NODE_ID_SIZE; i++)
	{
		Printf("%.2X ", mNodeId[i]);	
	}
}// END DebugPrint


u32 DistanceBetweenDhtNodes(const cDhtNodeId& lhs, const cDhtNodeId& rhs)
{
	u32 ret=0;
	for(u32 i=0; i < cDhtNodeId::NODE_ID_SIZE; i++)
	{
		ret += (lhs[i] ^ rhs[i]);
	}
	return ret;
}// END DistanceBetweenDhtNodes