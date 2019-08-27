#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "CSVParse.h"
#include "TriModalNormal.h"

typedef std::vector<double> vec;
typedef std::vector< std::vector<double> > matrix;

enum EFlowModel
{
	eNHM = 0,
	ePoisson,
	eCongested,
	eConstant
};

enum EVehicleModel
{
	eGrave = 0,
	eGarage
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
	virtual ~CModelData();

	virtual void ReadDataIn() = 0;

protected:
	std::string m_Path;
	CCSVParse m_CSV;
};

