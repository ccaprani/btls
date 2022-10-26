#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <memory>
#include "CSVParse.h"
#include "TriModalNormal.h"
#include "PyConfigData.h"

typedef std::vector<double> vec;
typedef std::vector< std::vector<double> > matrix;

enum EFlowModel
{
	eFM_NHM = 0,
	eFM_Poisson,
	eFM_Congested,
	eFM_Constant
};

enum EVehicleModel
{
	eVM_Grave = 0,
	eVM_Garage,
	eVM_Constant
};

struct Normal
{
	Normal(double m, double s)
	{
		Mean = m;
		StdDev = s;
	};
	Normal():Mean(0.0), StdDev(0.0){};
	double Mean;
	double StdDev;
};

class CModelData
{
public:
	CModelData();
	CModelData(CPyConfigData& pyConfig);
	virtual ~CModelData();

	virtual void ReadDataIn() = 0;

protected:
	std::filesystem::path m_Path;
	CCSVParse m_CSV;
};
typedef std::shared_ptr<CModelData> CModelData_sp;
