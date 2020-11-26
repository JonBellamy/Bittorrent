
// used to sort nodes by distance
class DistanceFuncObj_ForNodePtrs
{
public:
	DistanceFuncObj_ForNodePtrs(const cDhtNodeId& nodeId) : mNodeId(nodeId) {}
	bool operator() (const cDhtNode* lhs, const cDhtNode* rhs) 
	{ 
		return DistanceBetweenDhtNodes(lhs->Id(), mNodeId) < DistanceBetweenDhtNodes(rhs->Id(), mNodeId);
	}
	cDhtNodeId mNodeId;
};




class DistanceFuncObj_ForNodes
{
public:
	DistanceFuncObj_ForNodes(const cDhtNodeId& nodeId) : mNodeId(nodeId) {}
	bool operator() (const cDhtNode& lhs, const cDhtNode& rhs) 
	{ 
		return DistanceBetweenDhtNodes(lhs.Id(), mNodeId) < DistanceBetweenDhtNodes(rhs.Id(), mNodeId);
	}
	cDhtNodeId mNodeId;
};




// checks if the passed node is mMaxDistance or more further away from the member node
class Functor_NodeCloserThan
{
public:
	Functor_NodeCloserThan(u32 maxDistance, const cDhtNodeId& fromNode) : mFromNodeId(fromNode), mMaxDistance(maxDistance) {}

	bool operator() (const cDhtNode& node) 
	{ 
		return (DistanceBetweenDhtNodes(node.Id(), mFromNodeId) >= mMaxDistance);
	}

	cDhtNodeId mFromNodeId;
	u32 mMaxDistance;
};