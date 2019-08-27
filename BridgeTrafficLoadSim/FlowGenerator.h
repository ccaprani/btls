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
	
	size_t getCurBlock() {return m_CurBlock;};

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
	double m_TotalFlow;
	double m_TruckFlow;
	size_t m_CurBlock;		// i.e. which hour
	size_t m_BlockSize;		// Typically 3600 secs - an hour
	size_t m_BlockCount;	// Typically 24 blocks (hours)

	double m_BufferGapSpace;
	double m_BufferGapTime;

private:
	void updateBlock(double time);
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
	CFlowModelDataNHM* m_pFMD;
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
	CFlowModelDataCongested* m_pFMD;
	Normal m_Gap;
	double m_Speed;
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
	CFlowModelDataPoisson* m_pFMD;
	Normal m_Speed;
};

class CFlowGenConstant : public CFlowGenerator
{
public:
	CFlowGenConstant(CFlowModelData* pFDM) :CFlowGenerator(NULL,eFM_Constant){};
	virtual ~CFlowGenConstant(){};

protected:
	double GenerateGap(){ return 10.0; };		// MAGIC NUMBER - for testing
	double GenerateSpeed() { return 20.0; };;
	//void updateProperties();

private:
	//CFlowModelDataPoisson* m_pFMD;
	//Normal m_Speed;
};

