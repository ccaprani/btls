#pragma once
#include "ModelData.h"
#include "TrafficData.h"
#include "VehicleClassification.h"
#include "LaneFlowComposition.h"

class CVehicleModelData : public CModelData
{
public:
	CVehicleModelData(EVehicleModel vm, CVehicleClassification* pVC, CLaneFlowComposition lfc, const size_t nClass);
	virtual ~CVehicleModelData();

	EVehicleModel getModel() const { return m_Model; };
	const size_t getTruckClassCount() const { return m_TruckClassCount; };
	CVehicleClassification* getVehClassification() const { return m_pVehClassification; };
	double getLaneEccStd() const { return m_LaneEccStd; };
	vec getComposition(size_t i) const;

protected:	
	EVehicleModel m_Model;
	const size_t m_TruckClassCount;

	CVehicleClassification* m_pVehClassification;
	
	size_t m_NoLanes;
	size_t m_CurDirection;

	matrix m_mComposition;

	double m_LaneEccStd;

private:
};

