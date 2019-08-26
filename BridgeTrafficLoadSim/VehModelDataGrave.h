#pragma once
#include "VehicleModelData.h"

class CVehModelDataGrave : public CVehicleModelData
{
public:
	CVehModelDataGrave(CVehicleClassification* pVC, CLaneFlow lf);
	~CVehModelDataGrave();

	virtual void ReadDataIn();

	const CTrafficData& getTrafficData() const { return m_TD; };

private:
	void ReadFile_AW23();
	void ReadFile_AW45();
	void ReadFile_AS();
	void ReadFile_GVW();
	void ReadFile_ATW();	

	// could merge this in here at some later stage
	CTrafficData m_TD;
};

