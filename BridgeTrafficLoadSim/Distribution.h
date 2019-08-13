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

class CDistribution  
{
public:
	double GenerateNormal(double mean, double stdev);
	double GenerateTriModalNormal(CTriModalNormal TMN);
	double GenerateUniform();
	CDistribution();
	CDistribution(double loc, double sc, double sh);
	virtual ~CDistribution();

	void setShape(double sh);
	void setScale(double sc);
	void setLocation(double loc);

	double getShape() const;
	double getScale() const;
	double getLocation() const;
	
	double GenerateExponential();
	double GenerateLogNormal();
	double GenerateGamma();
	double GenerateGumbel();
	double GeneratePoisson();
	double GenerateGEV();
	double GenerateNormal();

private:
	double BoxMuller();
	
	static MTRand m_RNG;
	const double PI;

	double m_Location;
	double m_Scale;
	double m_Shape;
};

#endif // !defined(AFX_DISTRIBUTION_H__32235E3A_63B9_4BFC_B0CE_20A126A8B368__INCLUDED_)
