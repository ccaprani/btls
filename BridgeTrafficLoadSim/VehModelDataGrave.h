#pragma once
#include "VehicleModelData.h"

class CVehModelDataGrave : public CVehicleModelData
{
public:
	CVehModelDataGrave(CVehicleClassification_ptr pVC, CLaneFlowComposition lfc);
	~CVehModelDataGrave();

	virtual void ReadDataIn();

	std::vector<double>	GetGVWRange(int iTruck, int iRange);
	CTriModalNormal		GetSpacingDist(int iTruck, int iSpace);
	CTriModalNormal		GetAxleWeightDist(int iTruck, int iAxle);
	CTriModalNormal		GetTrackWidthDist(int iTruck, int iAxle);
	CTriModalNormal		GetGVW(int dir, int iTruck);

private:
	void ReadFile_AW23();
	void ReadFile_AW45();
	void ReadFile_AS();
	void ReadFile_GVW();
	void ReadFile_ATW();	

	// could merge this in here at some later stage
	CTrafficData m_TD;
};
typedef std::shared_ptr<CVehModelDataGrave> CVehModelDataGrave_ptr;

