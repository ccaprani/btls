// Distribution.h: interface for the CDistribution class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MultiModalNormal.h"
#include <random>

class CRNGWrapper
{
	static std::mt19937 m_MTgen;
	static std::uniform_real_distribution<double> m_Uniform;
	static std::normal_distribution<double> m_Norm;

public:
	double rand();
	double norm();

	/// Seed the process-wide Mersenne Twister generator.
	/// If never called, the generator is seeded from std::random_device
	/// at library load time (non-reproducible). Calling seed() makes
	/// subsequent random output deterministic for the given seed value.
	/// In multiprocessing contexts (spawn start method), each worker
	/// process has its own copy of the static generator, so calling
	/// seed(master_seed + worker_id) in each worker produces independent
	/// reproducible streams.
	static void seed(uint64_t s);
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
	double GenerateMultiModalNormal(CMultiModalNormal MMN);
	double GenerateExponential();
	double GenerateLogNormal();
	double GenerateGamma();
	double GenerateGumbel();
	double GeneratePoisson();
	double GenerateGEV();	
	double GenerateTriangular();
	double GenerateTriangular(double loc, double w);

private:
	double BoxMuller();
	
	CRNGWrapper m_RNG;

	const double PI;

	double m_Location;
	double m_Scale;
	double m_Shape;
};
