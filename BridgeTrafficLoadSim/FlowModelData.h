#pragma once
#include <vector>
#include "ModelData.h"
#include "LaneFlow.h"

class CFlowModelData :	public CModelData
{
public:
	CFlowModelData();
	virtual ~CFlowModelData();

	double getFlow(size_t iHour);
	double getCP_cars(size_t iHour);
	double getSpeedMean(int iHour);
	double getSpeedStd(int iHour);

protected:
	// Probably CLaneFlow needs to be merged into CFlowModelData
	// but we'll leave it alone for now
	CLaneFlow m_LaneFlow;

	double m_SpeedMean;
	double m_SpeedStd;
};

class CFlowModelDataNHM : public CFlowModelData
{
public:
	CFlowModelDataNHM();
	virtual ~CFlowModelDataNHM();

	void setNHM(matrix NHM);
	matrix GetNHM();

private:
	matrix m_vNHM;

};

class CFlowModelDataCongested : public CFlowModelData
{
public:
	CFlowModelDataCongested(double mean, double std);
	virtual ~CFlowModelDataCongested();

	void getGapParams(double& mean, double& std);

private:
	double m_GapMean;
	double m_GapStd;
};

class CFlowModelDataPoisson : public CFlowModelData
{
public:
	CFlowModelDataPoisson();
	virtual ~CFlowModelDataPoisson();
};

