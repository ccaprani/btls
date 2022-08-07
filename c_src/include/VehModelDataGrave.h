#pragma once
#include "VehicleModelData.h"

class CVehModelDataGrave : public CVehicleModelData
{
public:
	CVehModelDataGrave(CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	~CVehModelDataGrave();

	virtual void ReadDataIn();

	std::vector<double>	GetGVWRange(size_t iTruck, size_t iRange);
	CTriModalNormal		GetSpacingDist(size_t iTruck, size_t iSpace);
	CTriModalNormal		GetAxleWeightDist(size_t iTruck, size_t iAxle);
	CTriModalNormal		GetTrackWidthDist(size_t iTruck, size_t iAxle);
	CTriModalNormal		GetGVW(size_t dir, size_t iTruck);

private:
	void ReadFile_AW23();
	void ReadFile_AW45();
	void ReadFile_AS();
	void ReadFile_GVW();
	void ReadFile_ATW();	

	// could merge this in here at some later stage
	CTrafficData m_TD;
};
typedef std::shared_ptr<CVehModelDataGrave> CVehModelDataGrave_sp;

