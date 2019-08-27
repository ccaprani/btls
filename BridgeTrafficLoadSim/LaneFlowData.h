#pragma once
#include "ModelData.h"
#include "LaneFlowComposition.h"

class CLaneFlowData : public CModelData
{
public:
	CLaneFlowData();
	virtual ~CLaneFlowData();

	virtual void ReadDataIn();

	CLaneFlowComposition getLaneComp(size_t i) const;

	size_t getNoDirn() const { return m_NoDir;};
	size_t getNoLanes() const { return m_NoLanes; };
	size_t getNoLanesDir1() const { return m_NoLanesDir1; };
	size_t getNoLanesDir2() const { return m_NoLanesDir2; };

private:
	void ReadLaneFlow(std::string file);
	void SetRoadProperties();

	size_t m_NoLanes;
	size_t m_NoDir;
	size_t m_NoLanesDir1;
	size_t m_NoLanesDir2;

	size_t m_NoBlocks;	// typically hours
	size_t m_BlockSize;	// typically 3600 secs

	size_t m_TruckClassCount;

	std::vector<CLaneFlowComposition> m_vLaneComp;
};

