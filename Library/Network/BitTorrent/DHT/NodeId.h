// Jon Bellamy 18/11/2009


#ifndef DHT_NODE__ID_H_
#define DHT_NODE__ID_H_

#if USE_PCH
#include "stdafx.h"
#endif

#include <string>


#define INVALID_NODE_ID "00000000000000000000"


class cDhtNodeId
{
public:
    cDhtNodeId();
	cDhtNodeId(const std::string& data);
	cDhtNodeId(const cDhtNodeId& rhs);
	
	const cDhtNodeId& operator= (const cDhtNodeId& rhs);
	
	bool operator== (const cDhtNodeId& rhs) const;
	bool operator!= (const cDhtNodeId& rhs) const { return !(*this == rhs); }
	bool operator< (const cDhtNodeId& rhs) const;

	void Randomize();
	void FromData(const u8* data);

	bool IsValid() const;

	enum
	{
		NODE_ID_SIZE = 20
	};

	u8& operator[] (u32 index) { return mNodeId[index]; }
	u8 operator[] (u32 index) const { return mNodeId[index]; }

	std::string AsString() const;
	void DebugPrint() const;

private:

	friend class cDhtNode;

	u8 mNodeId[NODE_ID_SIZE];
};


typedef cDhtNodeId cDhtResourceId;


extern u32 DistanceBetweenDhtNodes(const cDhtNodeId& lhs, const cDhtNodeId& rhs);



#endif // DHT_NODE__ID_H_