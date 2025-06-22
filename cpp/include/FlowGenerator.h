#pragma once
#include <memory>
#include "Generator.h"
#include "FlowModelData.h"
#include "Vehicle.h"

//class CFlowGenerator; typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_sp;

class CFlowGenerator :	public CGenerator
{
public:
	CFlowGenerator(CFlowModelData_sp pFDM, EFlowModel fm);
	virtual ~CFlowGenerator();

	double Generate();

	virtual void prepareNextGen(double time, CVehicle_sp pPrevVeh, CVehicle_sp pNextVeh);

	void setMaxBridgeLength(double length);
	
	size_t getCurBlock() {return m_CurBlock;};

protected:
	virtual double GenerateGap() = 0;
	virtual double GenerateSpeed() = 0;
	virtual void updateProperties();
	double genExponential();
	
	CFlowModelData_sp m_pFlowModelData;
	EFlowModel m_FlowModel;
	CVehicle_sp m_pPrevVeh;
	CVehicle_sp m_pNextVeh;
	
	double m_MinGap;
	double m_TotalFlow;
	double m_TruckFlow;
	size_t m_CurBlock;		// i.e. which hour
	size_t m_BlockSize;		// Typically 3600 secs - an hour
	size_t m_BlockCount;	// Typically 24 blocks (hours)

	double m_MaxBridgeLength;
	double m_BufferGapSpace;
	double m_BufferGapTime;

private:
	void updateBlock(double time);
	void updateExponential();
	void setMinGap();
};
typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_sp;

class CFlowGenHeDS : public CFlowGenerator
{
public:
	CFlowGenHeDS(CFlowModelDataHeDS_sp pFDM);
	virtual ~CFlowGenHeDS();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataHeDS_sp m_pFMD;
	matrix m_vHeDS;
	Normal m_Speed;
};
typedef std::shared_ptr<CFlowGenHeDS> CFlowGenHeDS_sp;

class CFlowGenCongested : public CFlowGenerator
{
public:
	CFlowGenCongested(CFlowModelDataCongested_sp pFDM);
	virtual ~CFlowGenCongested();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataCongested_sp m_pFMD;
	Normal m_Gap;
	double m_Speed;
};
typedef std::shared_ptr<CFlowGenCongested> CFlowGenCongested_sp;

class CFlowGenPoisson : public CFlowGenerator
{
public:
	CFlowGenPoisson(CFlowModelDataPoisson_sp pFDM);
	virtual ~CFlowGenPoisson();

protected:
	double GenerateGap();
	double GenerateSpeed();
	void updateProperties();

private:
	CFlowModelDataPoisson_sp m_pFMD;
	Normal m_Speed;
};
typedef std::shared_ptr<CFlowGenPoisson> CFlowGenPoisson_sp;


class CFlowGenConstant : public CFlowGenerator
{
public:
	CFlowGenConstant(CFlowModelDataConstant_sp pFDM);
	virtual ~CFlowGenConstant();

protected:
	double GenerateGap();
	double GenerateSpeed();

private:
	CFlowModelDataConstant_sp m_pFMD;
};
typedef std::shared_ptr<CFlowGenConstant> CFlowGenConstant_sp;

