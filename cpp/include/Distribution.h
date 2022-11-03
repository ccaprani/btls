// Distribution.h: interface for the CDistribution class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "TriModalNormal.h"
#include <random>

class CRNGWrapper
{
	static std::mt19937 m_MTgen;
	static std::uniform_real_distribution<double> m_Uniform;
	static std::normal_distribution<double> m_Norm;

public:
	double rand();
	double norm();
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
	double GenerateTriangular();

private:
	double BoxMuller();
	
	CRNGWrapper m_RNG;

	const double PI;

	double m_Location;
	double m_Scale;
	double m_Shape;
};
