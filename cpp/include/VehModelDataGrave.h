#pragma once
#include "VehicleModelData.h"
#include "TriModalNormal.h"

class CVehModelDataGrave : public CVehicleModelData
{
public:
	CVehModelDataGrave(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	~CVehModelDataGrave();

	virtual void ReadDataIn();

	std::vector<double>	GetGVWRange(size_t iTruck, size_t iRange);
	CTriModalNormal		GetSpacingDist(size_t iTruck, size_t iSpace);
	CTriModalNormal		GetAxleWeightDist(size_t iTruck, size_t iAxle);
	CTriModalNormal		GetTrackWidthDist(size_t iTruck, size_t iAxle);
	CTriModalNormal		GetGVW(size_t dir, size_t iTruck);

	void Add2AxleSpacings(std::vector<CTriModalNormal> vSpace);
	void Add3AxleSpacings(std::vector<CTriModalNormal> vSpace);
	void Add4AxleSpacings(std::vector<CTriModalNormal> vSpace);
	void Add5AxleSpacings(std::vector<CTriModalNormal> vSpace);

	void Add2AxleTrackWidth(std::vector<CTriModalNormal> vSpace);
	void Add3AxleTrackWidth(std::vector<CTriModalNormal> vSpace);
	void Add4AxleTrackWidth(std::vector<CTriModalNormal> vSpace);
	void Add5AxleTrackWidth(std::vector<CTriModalNormal> vSpace);

	void Add2AxleWeight(std::vector<CTriModalNormal> vAxle);
	void Add3AxleWeight(std::vector<CTriModalNormal> vAxle);
	void Add45AxleWeight(std::vector<double> data, std::size_t iTruck, std::size_t iRange);
	
	void AddGVW(int dir, std::vector<CTriModalNormal> vGVW);
	
	// Speed is kept here as part of the class modelling
	void AddSpeed(std::vector<CTriModalNormal> vSpeed);	
	CTriModalNormal			GetSpeed(std::size_t dir);

private:
	void ReadFile_AW23();
	void ReadFile_AW45();
	void ReadFile_AS();
	void ReadFile_GVW();
	void ReadFile_ATW();	

	std::vector<CTriModalNormal> m_v2AxleSpacings;
	std::vector<CTriModalNormal> m_v3AxleSpacings;
	std::vector<CTriModalNormal> m_v4AxleSpacings;
	std::vector<CTriModalNormal> m_v5AxleSpacings;

	std::vector<CTriModalNormal> m_v2AxleTrackWidth;
	std::vector<CTriModalNormal> m_v3AxleTrackWidth;
	std::vector<CTriModalNormal> m_v4AxleTrackWidth;
	std::vector<CTriModalNormal> m_v5AxleTrackWidth;

	std::vector<CTriModalNormal> m_v2AxleWeight;
	std::vector<CTriModalNormal> m_v3AxleWeight;

	std::vector<CTriModalNormal> m_vDir1GVW;
	std::vector<CTriModalNormal> m_vDir2GVW;
	std::vector<CTriModalNormal> m_vSpeed;

	struct Dist
	{
		double Mean;
		double StdDev;
	};

	struct GVWRange
	{
		Dist W1;
		Dist W2;
		Dist WT;
	};
	
	std::vector<GVWRange> m_v4AxleWeight;
	std::vector<GVWRange> m_v5AxleWeight;
};
typedef std::shared_ptr<CVehModelDataGrave> CVehModelDataGrave_sp;

