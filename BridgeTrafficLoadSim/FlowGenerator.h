#pragma once
#include "Generator.h"
#include "FlowModelData.h"
#include "Vehicle.h"

//class CFlowGenerator; typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_ptr;

class CFlowGenerator :	public CGenerator
{
public:
	CFlowGenerator(CFlowModelData_ptr pFDM, EFlowModel fm);
	virtual ~CFlowGenerator();

	double Generate();

	virtual void prepareNextGen(double time, CVehicle_ptr pPrevVeh, CVehicle_ptr pNextVeh);
	
	size_t getCurBlock() {return m_CurBlock;};

protected:
	virtual double GenerateGap() = 0;
	virtual double GenerateSpeed() = 0;
	virtual void updateProperties();
	double genExponential();
	
	EFlowModel m_FlowModel;
	CFlowModelData_ptr m_pFlowModelData;
	CVehicle_ptr m_pPrevVeh;
	CVehicle_ptr m_pNextVeh;
	
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
typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_ptr;

class CFlowGenNHM : public CFlowGenerator
{
public:
	CFlowGenNHM(CFlowModelDataNHM_ptr pFDM);
	virtual ~CFlowGenNHM();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataNHM_ptr m_pFMD;
	matrix m_vNHM;
	Normal m_Speed;
};
typedef std::shared_ptr<CFlowGenNHM> CFlowGenNHM_ptr;

class CFlowGenCongested : public CFlowGenerator
{
public:
	CFlowGenCongested(CFlowModelDataCongested_ptr pFDM);
	virtual ~CFlowGenCongested();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataCongested_ptr m_pFMD;
	Normal m_Gap;
	double m_Speed;
};
typedef std::shared_ptr<CFlowGenCongested> CFlowGenCongested_ptr;

class CFlowGenPoisson : public CFlowGenerator
{
public:
	CFlowGenPoisson(CFlowModelDataPoisson_ptr pFDM);
	virtual ~CFlowGenPoisson();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataPoisson_ptr m_pFMD;
	Normal m_Speed;
};
typedef std::shared_ptr<CFlowGenPoisson> CFlowGenPoisson_ptr;

class CFlowGenConstant : public CFlowGenerator
{
public:
	CFlowGenConstant(CFlowModelData_ptr pFDM) : CFlowGenerator(nullptr, eFM_Constant){};
	virtual ~CFlowGenConstant(){};

protected:
	double GenerateGap(){ return 10.0; };		// MAGIC NUMBER - for testing
	double GenerateSpeed() { return 20.0; };;
	//void updateProperties();

private:
	//CFlowModelDataPoisson_ptr m_pFMD;
	//Normal m_Speed;
};
typedef std::shared_ptr<CFlowGenConstant> CFlowGenConstant_ptr;

