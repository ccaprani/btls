#pragma once
#include <vector>
#include "ModelData.h"
#include "LaneFlow.h"

class CFlowModelData :	public CModelData
{
public:
	CFlowModelData(CLaneFlow lf, EFlowModel fm);
	virtual ~CFlowModelData();

	//virtual void ReadDataIn();

	void	setLane(size_t indx);
	double	getFlow(size_t iHour);
	double	getCP_cars(size_t iHour);
	void	getSpeedParams(size_t iHour, double& mean, double& std);
	void	getGapBuffers(double& space, double& time);

	EFlowModel getModel() const { return m_Model; };

protected:
	EFlowModel m_Model;

	// Probably CLaneFlow needs to be merged into CFlowModelData
	// but we'll leave it alone for now
	CLaneFlow m_LaneFlow;

	double m_SpeedMean;
	double m_SpeedStd;

	double m_BufferGapSpace;
	double m_BufferGapTime;

private:
	std::vector<CLaneFlow> ReadLaneFlow(std::string file);
};

class CFlowModelDataNHM : public CFlowModelData
{
public:
	CFlowModelDataNHM(CLaneFlow lf);
	virtual ~CFlowModelDataNHM();

	virtual void ReadDataIn();

	matrix GetNHM();

private:
	void ReadFile_NHM();
	//vec ReadLaneFlow(std::string file);
	matrix m_vNHM;

};

class CFlowModelDataCongested : public CFlowModelData
{
public:
	CFlowModelDataCongested(CLaneFlow lf);
	virtual ~CFlowModelDataCongested();

	virtual void ReadDataIn();

	double getSpeed() const { return m_Speed; };
	void getGapParams(double& mean, double& std);

private:
	double m_Speed;
	double m_GapMean;
	double m_GapStd;
};

class CFlowModelDataPoisson : public CFlowModelData
{
public:
	CFlowModelDataPoisson(CLaneFlow lf);
	virtual ~CFlowModelDataPoisson();

	virtual void ReadDataIn();
};

