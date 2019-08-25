#pragma once
#include "Generator.h"
#include "FlowModelData.h"

enum EFlowModel
{
	eNHM = 0,
	ePoisson,
	eCongested
};

//class CFlowGenerator; typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_ptr;

class CFlowGenerator :	public CGenerator
{
public:
	CFlowGenerator(EFlowModel fm);
	virtual ~CFlowGenerator();

	virtual double Generate() = 0;
	
	void updateHour(double time);
	size_t getHour() {return m_CurHour;};

protected:
	virtual double GenerateSpeed() = 0;
	virtual void updateProperties() = 0;
	
	EFlowModel m_FlowModel;
	CFlowModelData* m_pFlowModelData;
	
	double m_vFlowRate;
	double m_CarPerc;
	size_t m_CurHour;

private:
	void updateFlowProperties();
};

class CFlowGenNHM : public CFlowGenerator
{
public:
	CFlowGenNHM(CFlowModelDataNHM* pFDM);
	virtual ~CFlowGenNHM();

	double Generate();

protected:
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataNHM* m_pFDM;
};

class CFlowGenCongested : public CFlowGenerator
{
public:
	CFlowGenCongested(CFlowModelDataCongested* pFDM);
	virtual ~CFlowGenCongested();

	double Generate();

protected:
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataCongested* m_pFDM;
	double m_GapMean;
	double m_GapStd;
};

class CFlowGenPoisson : public CFlowGenerator
{
public:
	CFlowGenPoisson(CFlowModelDataPoisson* pFDM);
	virtual ~CFlowGenPoisson();

	double Generate();

protected:
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataPoisson* m_pFDM;
};

