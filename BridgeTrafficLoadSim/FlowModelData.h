#pragma once
#include <vector>
#include "ModelData.h"
#include "LaneFlow.h"

class CFlowModelData :	public CModelData
{
public:
	CFlowModelData(CLaneFlow lf, EFlowModel fm);
	virtual ~CFlowModelData();

	double	getFlow(size_t iHour);
	double	getCP_cars(size_t iHour);
	void	getSpeedParams(size_t iHour, double& mean, double& std);
	void	getGapBuffers(double& space, double& time);

	EFlowModel getFlowModel() const { return m_FlowModel; };

protected:
	EFlowModel m_FlowModel;

	// Probably CLaneFlow needs to be merged into CFlowModelData
	// but we'll leave it alone for now
	CLaneFlow m_LaneFlow;

	double m_SpeedMean;
	double m_SpeedStd;

	double m_BufferGapSpace;
	double m_BufferGapTime;
};

class CFlowModelDataNHM : public CFlowModelData
{
public:
	CFlowModelDataNHM(CLaneFlow lf, matrix vNHM);
	virtual ~CFlowModelDataNHM();

	matrix GetNHM();

private:
	matrix m_vNHM;

};

class CFlowModelDataCongested : public CFlowModelData
{
public:
	CFlowModelDataCongested(CLaneFlow lf, double mean, double std);
	virtual ~CFlowModelDataCongested();

	void getGapParams(double& mean, double& std);

private:
	double m_GapMean;
	double m_GapStd;
};

class CFlowModelDataPoisson : public CFlowModelData
{
public:
	CFlowModelDataPoisson(CLaneFlow lf);
	virtual ~CFlowModelDataPoisson();
};

