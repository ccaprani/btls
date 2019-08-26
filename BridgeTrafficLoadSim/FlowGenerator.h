#pragma once
#include "Generator.h"
#include "FlowModelData.h"
#include "Vehicle.h"

//class CFlowGenerator; typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_ptr;

class CFlowGenerator :	public CGenerator
{
public:
	CFlowGenerator(CFlowModelData* pFDM, EFlowModel fm);
	virtual ~CFlowGenerator();

	double Generate();

	virtual void prepareNextGen(double time, CVehicle* pPrevVeh, CVehicle* pNextVeh);
	
	size_t getHour() {return m_CurHour;};

protected:
	virtual double GenerateGap() = 0;
	virtual double GenerateSpeed() = 0;
	virtual void updateProperties();
	double genExponential();
	
	EFlowModel m_FlowModel;
	CFlowModelData* m_pFlowModelData;
	CVehicle* m_pPrevVeh;
	CVehicle* m_pNextVeh;
	
	double m_MinGap;
	double m_vFlowRate;
	double m_CarPerc;
	size_t m_CurHour;

	double m_BufferGapSpace;
	double m_BufferGapTime;

private:
//	void updateFlowProperties();
	void updateHour(double time);
	void updateExponential();
	void setMinGap();
};

class CFlowGenNHM : public CFlowGenerator
{
public:
	CFlowGenNHM(CFlowModelDataNHM* pFDM);
	virtual ~CFlowGenNHM();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataNHM* m_pFDM;
	matrix m_vNHM;
	Normal m_Speed;
};

class CFlowGenCongested : public CFlowGenerator
{
public:
	CFlowGenCongested(CFlowModelDataCongested* pFDM);
	virtual ~CFlowGenCongested();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataCongested* m_pFDM;
	Normal m_Gap;
	Normal m_Speed;
};

class CFlowGenPoisson : public CFlowGenerator
{
public:
	CFlowGenPoisson(CFlowModelDataPoisson* pFDM);
	virtual ~CFlowGenPoisson();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataPoisson* m_pFDM;
	Normal m_Speed;
};

