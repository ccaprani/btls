#pragma once
#include <memory>
#include <vector>
#include "ModelData.h"
#include "LaneFlowComposition.h"

class CFlowModelData :	public CModelData
{
public:
	CFlowModelData(CConfigDataCore& config, EFlowModel fm, CLaneFlowComposition lfc, const bool bCars);
	virtual ~CFlowModelData();

	vec		getCarPercent() const { return m_vCarPercent; };
	void	getFlow(size_t i, double& totalFlow, double& truckFlow);
	void	getSpeedParams(size_t i, Normal& speed);
	void	getGapLimits(double& bridge, double& space, double& time);

	EFlowModel getModel() const { return m_Model; };
	bool getModelHasCars() const { return m_bModelHasCars; };

	void getBlockInfo(size_t& sz, size_t& n) const;

protected:
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

class CFlowModelDataHeDS : public CFlowModelData
{
public:
	CFlowModelDataHeDS(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataHeDS();

	virtual void ReadDataIn();

	matrix GetHeDS();

private:
	void ReadFile_HeDS();
	//vec ReadLaneFlow(std::string file);
	matrix m_vHeDS;

};
typedef std::shared_ptr<CFlowModelDataHeDS> CFlowModelDataHeDS_sp;

class CFlowModelDataCongested : public CFlowModelData
{
public:
	CFlowModelDataCongested(CConfigDataCore& config, CLaneFlowComposition lfc);
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
	CFlowModelDataPoisson(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataPoisson();

	virtual void ReadDataIn();
};
typedef std::shared_ptr<CFlowModelDataPoisson> CFlowModelDataPoisson_sp;

class CFlowModelDataConstant : public CFlowModelData
{
public:
	CFlowModelDataConstant(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataConstant();

	double getConstSpeed();
	double getConstGap();

	virtual void ReadDataIn();

private:
	double m_ConstSpeed;
	double m_ConstGap;
};
typedef std::shared_ptr<CFlowModelDataConstant> CFlowModelDataConstant_sp;

