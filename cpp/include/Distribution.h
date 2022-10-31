// Distribution.h: interface for the CDistribution class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DISTRIBUTION_H__32235E3A_63B9_4BFC_B0CE_20A126A8B368__INCLUDED_)
#define AFX_DISTRIBUTION_H__32235E3A_63B9_4BFC_B0CE_20A126A8B368__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MersenneTwister.h"
#include "TriModalNormal.h"
#include <random>

class CRNGWrapper
{
	//static MTRand m_RNG;
	static std::mt19937 m_MTgen;
	static std::uniform_real_distribution<double> m_RNG;

public:
	double rand();
};

class CDistribution  
{
public:
	CDistribution();
	CDistribution(double loc, double sc, double sh);
	virtual ~CDistribution();

	void setShape(double sh);
	void setScale(double sc);
	void setLocation(double loc);

	double getShape() const;
	double getScale() const;
	double getLocation() const;

	double GenerateUniform();
	double GenerateNormal();	
	double GenerateNormal(double mean, double stdev);
	double GenerateTriModalNormal(CTriModalNormal TMN);
	double GenerateExponential();
	double GenerateLogNormal();
	double GenerateGamma();
	double GenerateGumbel();
	double GeneratePoisson();
	double GenerateGEV();	

private:
	double BoxMuller();
	
	CRNGWrapper m_RNG;

	const double PI;

	double m_Location;
	double m_Scale;
	double m_Shape;
};

#endif // !defined(AFX_DISTRIBUTION_H__32235E3A_63B9_4BFC_B0CE_20A126A8B368__INCLUDED_)
