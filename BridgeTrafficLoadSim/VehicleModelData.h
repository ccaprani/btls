#pragma once
#include "ModelData.h"
#include "TrafficData.h"
#include "VehicleClassification.h"

class CVehicleModelData : public CModelData
{
public:
	CVehicleModelData(CVehicleClassification* pVC, CLaneFlow lf, EVehicleModel vm);
	virtual ~CVehicleModelData();

	EVehicleModel getModel() const { return m_Model; };
	CVehicleClassification* getVehClassification() const { return m_pVehClassification; };
	CLaneFlow getLaneFlow() const { return m_LaneFlow; };
	double getLaneEccStd() const { return m_LaneEccStd; };

protected:	
	EVehicleModel m_Model;

	CVehicleClassification* m_pVehClassification;
	
	// Not sure why we should need the flow data in here
	CLaneFlow m_LaneFlow;

	size_t m_NoLanes;
	size_t m_CurDirection;

	double m_LaneEccStd;

private:
};

