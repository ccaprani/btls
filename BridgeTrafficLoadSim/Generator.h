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

	struct Normal
	{
		double Mean;
		double StdDev;
	};

	CModelData* m_pModelData;
	CDistribution m_RNG;
};

