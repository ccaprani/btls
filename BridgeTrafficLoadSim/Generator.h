#pragma once
#include "ModelData.h"
#include "Distribution.h"

class CGenerator
{
public:
	CGenerator();
	virtual ~CGenerator();

protected:
	CModelData* m_pModelData;

	CDistribution m_RNG;
};

