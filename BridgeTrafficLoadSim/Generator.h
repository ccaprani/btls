#pragma once
#include "ModelData.h"
#include "Distribution.h"

class CGenerator
{
public:
	CGenerator();
	virtual ~CGenerator();

protected:
	double	inv_quad(double a, double b, double c, double y);
	double	quad(double a, double b, double c, double x);
	void	ScaleVector(std::vector<double>& vec, double scale);
	double	SumVector(std::vector<double> vec);

	struct Normal
	{
		double Mean;
		double StdDev;
	};

	CModelData* m_pModelData;
	CDistribution m_RNG;
};

