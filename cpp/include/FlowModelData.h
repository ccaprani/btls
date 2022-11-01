#pragma once
#include <memory>
#include <vector>
#include "ModelData.h"
#include "LaneFlowComposition.h"

class CFlowModelData :	public CModelData
{
public:
	CFlowModelData(EFlowModel fm, CLaneFlowComposition lfc, const bool bCars, CConfigDataCore& config);
	virtual ~CFlowModelData();

	vec		getCarPercent() const { return m_vCarPercent; };
	void	getFlow(size_t i, double& totalFlow, double& truckFlow);
	void	getSpeedParams(size_t i, Normal& speed);
	void	getGapLimits(double& bridge, double& space, double& time);

	EFlowModel getModel() const { return m_Model; };
	const bool getModelHasCars() const { return m_bModelHasCars; };

	void getBlockInfo(size_t& sz, size_t& n) const;

protected:
	CConfigDataCore& m_Config;

	EFlowModel m_Model;
	const bool m_bModelHasCars;

	vec m_vTotalFlow;
	vec m_vTruckFlow;
	vec m_vCarPercent;
	std::vector<Normal> m_vSpeed;

	size_t m_BlockSize;		// Typically 3600 secs - an hour
	size_t m_BlockCount;	// Typically 24 blocks (hours)

	double m_SpeedMean;
	double m_SpeedStd;

	const double m_BufferGapSpace;
	const double m_BufferGapTime;

private:

};
typedef std::shared_ptr<CFlowModelData> CFlowModelData_sp;

class CFlowModelDataNHM : public CFlowModelData
{
public:
	CFlowModelDataNHM(CLaneFlowComposition lfc);
	CFlowModelDataNHM(CLaneFlowComposition lfc, CConfigDataCore& config);
	virtual ~CFlowModelDataNHM();

	virtual void ReadDataIn();

	matrix GetNHM();

private:
	void ReadFile_NHM();
	//vec ReadLaneFlow(std::string file);
	matrix m_vNHM;

};
typedef std::shared_ptr<CFlowModelDataNHM> CFlowModelDataNHM_sp;

class CFlowModelDataCongested : public CFlowModelData
{
public:
	CFlowModelDataCongested(CLaneFlowComposition lfc);
	CFlowModelDataCongested(CLaneFlowComposition lfc, CConfigDataCore& config);
	virtual ~CFlowModelDataCongested();

	virtual void ReadDataIn();

	double getSpeed() const { return m_Speed; };
	void getGapParams(double& mean, double& std);

private:
	double m_Speed;
	double m_GapMean;
	double m_GapStd;
};
typedef std::shared_ptr<CFlowModelDataCongested> CFlowModelDataCongested_sp;

class CFlowModelDataPoisson : public CFlowModelData
{
public:
	CFlowModelDataPoisson(CLaneFlowComposition lfc);
	CFlowModelDataPoisson(CLaneFlowComposition lfc, CConfigDataCore& config);
	virtual ~CFlowModelDataPoisson();

	virtual void ReadDataIn();
};
typedef std::shared_ptr<CFlowModelDataPoisson> CFlowModelDataPoisson_sp;

class CFlowModelDataConstant : public CFlowModelData
{
public:
	CFlowModelDataConstant(CLaneFlowComposition lfc);
	CFlowModelDataConstant(CLaneFlowComposition lfc, CConfigDataCore& config);
	virtual ~CFlowModelDataConstant();

	double getConstSpeed();
	double getConstGap();

	virtual void ReadDataIn();

private:
	double m_ConstSpeed;
	double m_ConstGap;
};
typedef std::shared_ptr<CFlowModelDataConstant> CFlowModelDataConstant_sp;

