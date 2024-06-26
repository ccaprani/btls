#pragma once
#include "ModelData.h"
#include "LaneFlowComposition.h"
#include <memory>
// forward declare
class CVehicle; typedef std::shared_ptr<CVehicle> CVehicle_sp;

class CLaneFlowData : public CModelData
{
public:
	CLaneFlowData(CConfigDataCore& config);
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
	// void ReadGarageFile();
	
	size_t m_NoLanes;
	size_t m_NoDir;
	size_t m_NoLanesDir1;
	size_t m_NoLanesDir2;

	size_t m_TruckClassCount;
	size_t m_BlockSize;	// typically 3600 secs
	size_t m_NoBlocks;	// typically hours

	std::vector<CLaneFlowComposition> m_vLaneComp;

	std::vector<CVehicle_sp> m_vVehicles;
};
typedef std::shared_ptr<CLaneFlowData> CLaneFlowData_sp;
