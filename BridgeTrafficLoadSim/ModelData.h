#pragma once
#include <vector>

typedef std::vector<double> vec;
typedef std::vector< std::vector<double> > matrix;

enum EFlowModel
{
	eNHM = 0,
	ePoisson,
	eCongested
};

class CModelData
{
public:
	CModelData();
	virtual ~CModelData();
};

