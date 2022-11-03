// Distribution.cpp: implementation of the CDistribution class.
//
//////////////////////////////////////////////////////////////////////

#include "Distribution.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Initialise static member of class - must be in source file
std::uniform_real_distribution<double> CRNGWrapper::m_Uniform(0.0, 1.0);
std::normal_distribution<double> CRNGWrapper::m_Norm(0.0,1.0);
//std::mt19937 CRNGWrapper::m_MTgen(100); // MAGIC NUMBER - constant seed for debugging
std::mt19937 CRNGWrapper::m_MTgen(std::random_device{}());

inline double CRNGWrapper::rand() 
{
	return m_Uniform(m_MTgen);
};

inline double CRNGWrapper::norm()
{
	return m_Norm(m_MTgen);
};

CDistribution::CDistribution():PI(3.14159265359)
{
	m_Location = 10.0;
	m_Scale = 2.0;
	m_Shape = 0.0;
}

CDistribution::CDistribution(double loc, double sc, double sh):PI(3.14159265359)
{
	m_Location = loc;
	m_Scale = sc;
	m_Shape = sh;
}

CDistribution::~CDistribution()
{

}

double CDistribution::getLocation() const
{
	return m_Location;
}	

double CDistribution::getScale() const
{
	return m_Scale;
}

double CDistribution::getShape() const
{
	return m_Shape;
}

void CDistribution::setLocation(double loc)
{
	m_Location = loc;
}

void CDistribution::setScale(double sc)
{
	m_Scale = sc;
}

void CDistribution::setShape(double sh)
{
	m_Shape = sh;
}

///////////////////////////////////////////////////
// The random deviate generators

double CDistribution::GenerateExponential()
{
	double u01 = m_RNG.rand();
	double val = m_Location - m_Scale * log(u01);
	return val;
}

double CDistribution::GenerateLogNormal()
{
	double x[2], u[2];
	u[0] = m_RNG.rand();
	u[1] = m_RNG.rand();
	x[0] = m_Location + m_Scale * sqrt(-2 * log(u[0])) * cos(2 * PI * u[1]);
	x[1] = m_Location + m_Scale * sqrt(-2 * log(u[0])) * sin(2 * PI * u[1]);
	double val = exp( x[ (int)(u[1] + 0.5) ] );

	return val;
}

double CDistribution::GenerateGamma()
{
	double u[3];
	u[0] = m_RNG.rand();
	u[1] = m_RNG.rand();
	u[2] = m_RNG.rand();
	double val = (1/m_Location) 
					* (-log(u[2])) 
					* pow( u[0], (1/m_Scale) ) 
					/ pow( u[0], (1/m_Scale) 
					+ pow( u[1], (1/(1 - m_Scale)) ) );
	
	return val;
}

double CDistribution::GenerateGumbel()
{
	double u01 = m_RNG.rand();
	double val = m_Location - (1 / m_Scale) * log(log(1 / u01));

	return val;
}

double CDistribution::GeneratePoisson()
{
	double x[2], u[2];
	u[0] = m_RNG.rand();
	u[1] = m_RNG.rand();

	double m = m_Location - 0.5;
	double sigma = sqrt(m_Scale);

	x[0] = m + sigma * sqrt(-2 * log(u[0])) * cos(2 * PI * u[1]);
	x[1] = m + sigma * sqrt(-2 * log(u[0])) * sin(2 * PI * u[1]);
	double val = x[ (int)(u[1] + 0.5) ];

	return val;
}

double CDistribution::GenerateGEV()
{
	double u01 = m_RNG.rand();
	double val = m_Location + m_Scale * (1 - pow(-log(u01),m_Shape)) / m_Shape;

	return val;
}

double CDistribution::GenerateNormal()
{
	//return m_Location + m_Scale * BoxMuller();
	return m_Location + m_Scale * m_RNG.norm();
}

double CDistribution::GenerateNormal(double mean, double stdev)
{
	//return mean + stdev * BoxMuller();
	return mean + stdev * m_RNG.norm();
}

double CDistribution::BoxMuller()
{
	// normal random variate generator
	
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if (use_last)		        // use value from previous call
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0 * m_RNG.rand() - 1.0;
			x2 = 2.0 * m_RNG.rand() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}
	return( y1 );
}

double CDistribution::GenerateUniform()
{
	return m_RNG.rand();
}

double CDistribution::GenerateTriModalNormal(CTriModalNormal TMN)
{
	double mode = m_RNG.rand();
	double val = 0.0;

	double mean, stdev;

	if(mode <= TMN.m_vModes[0].Weight)
	{
		mean = TMN.m_vModes[0].Mean;		stdev = TMN.m_vModes[0].StdDev;
		//val = GenerateNormal(TMN.m_vModes[0].Mean, TMN.m_vModes[0].StdDev);
	}
	else if( (mode > TMN.m_vModes[0].Weight) && (mode <= (TMN.m_vModes[1].Weight + TMN.m_vModes[0].Weight) ) )
	{
		mean = TMN.m_vModes[1].Mean;		stdev = TMN.m_vModes[1].StdDev;
		//val = GenerateNormal(TMN.m_vModes[1].Mean, TMN.m_vModes[1].StdDev);
	}
	else
	{
		mean = TMN.m_vModes[2].Mean;		stdev = TMN.m_vModes[2].StdDev;
		//val = GenerateNormal(TMN.m_vModes[2].Mean, TMN.m_vModes[2].StdDev);
	}

	val = GenerateNormal(mean, stdev);
	return val;
}

double CDistribution::GenerateTriangular()
{
	double u = m_RNG.rand();
	double& w = m_Scale;
	double val = 0.0;

	if(u < 0.5)
	{
		val = w*(sqrt(2*u)-1);
	}
	else
	{
		val = w*(1 - sqrt(2*(1-u)));
	}
	return val + m_Location;
}
