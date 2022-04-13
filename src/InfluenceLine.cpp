#include "InfluenceLine.h"

using namespace std;

CInfluenceLine::CInfluenceLine(void)
	: m_Type(0), m_Weight(1.0)
{
	// Type: 1 - expression, 2 - discrete, 3 - Surface

	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect1);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect2);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect3);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect4);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect5);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect6);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect7);
}


CInfluenceLine::~CInfluenceLine(void)
{
}

CInfluenceSurface* CInfluenceLine::getIS()
{
	return &m_IS;
}

// get the load effect value given an axle vector
double CInfluenceLine::getLoadEffect(std::vector<CAxle>& vAxles)
{
	double effVal = 0.0;
	for(unsigned int j = 0; j < vAxles.size(); j++)
		effVal += getAxleLoadEffect(vAxles[j]);
	return effVal;
}

double CInfluenceLine::getAxleLoadEffect(CAxle& axle)
{
	double effVal = 0.0;

	if(m_Type == 3)	// Influence surface
	{
		double ord1 = m_IS.giveOrdinate(axle.m_Position,axle.m_Eccentricity-axle.m_TrackWidth/2,axle.m_Lane);
		double ord2 = m_IS.giveOrdinate(axle.m_Position,axle.m_Eccentricity+axle.m_TrackWidth/2,axle.m_Lane);
		effVal = 0.5*axle.m_AxleWeight*(ord1+ord2); // assumes half axle weight on each wheel		
	}
	else
		effVal = axle.m_AxleWeight*getOrdinate(axle.m_Position);
	
	return effVal;
}


// get the ordinate at the position x
double CInfluenceLine::getOrdinate(double x)
{
	double ord = 0.0;
	if(m_Type == 2)
		ord = getDiscreteOrdinate(x);
	else
		ord = getEquationOrdinate(x);
	return ord * m_Weight;
}


// set the influence line no.
void CInfluenceLine::setIL(size_t nIL, double length)
{
	m_Type = 1;
	m_ILFunctionNo = nIL;
	m_Length = length;
	// set the function pointer to the correct LE equation
	m_LEfptr = m_vLEfptr.at(m_ILFunctionNo-1); // -1 zero based array
}

// set the influence line data
void CInfluenceLine::setIL(vector<double> vDis, vector<double> vOrd)
{
	m_Type = 2;
	m_vDistance = vDis;
	m_vOrdinate = vOrd;
	m_NoPoints = m_vDistance.size();
	m_Length = m_vDistance.at(m_NoPoints-1);
}

// Set the influence surface
void CInfluenceLine::setIL(CInfluenceSurface IS)
{
	m_Type = 3;
	m_IS = IS;
	m_Length = m_IS.getLength();
}

double CInfluenceLine::getLength(void)
{
	return m_Length;
}


void CInfluenceLine::setIndex(size_t n)
{
	m_Index = n;
}

size_t CInfluenceLine::getIndex(void)
{
	return m_Index;
}

void CInfluenceLine::setWeight(double weight)
{
	m_Weight = weight;
}

double CInfluenceLine::getDiscreteOrdinate(double x)
{
	int i = 0;
	// right at the end of the IL
	if(x >= m_Length - 0.001 && x <= m_Length + 0.001)
		return m_vOrdinate.at(m_NoPoints-1);
	// not on the IL
	else if(x < m_vDistance.at(0) || x > m_Length)
		return 0.0;
	// On the IL, but not at the end
	else
	{
		while(x >= m_vDistance.at(i)) i++;	// find the index
		double deltaX = m_vDistance.at(i) - m_vDistance.at(i-1);
		double ord1 = m_vOrdinate.at(i-1);
		double ord2 = m_vOrdinate.at(i);
		double ordinate = ord1 + (x-m_vDistance.at(i-1))/deltaX*(ord2-ord1);
		return ordinate;
	}
}

double CInfluenceLine::getEquationOrdinate(double x)
{
	// using function pointers to select correct LE equation
	return (this->*m_LEfptr)(x);
}

size_t CInfluenceLine::getNoPoints(void)
{
	return m_NoPoints;
}

// Mid-span bending moment: simply-supported beam
double CInfluenceLine::LoadEffect1(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;
	
	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = x * (1 - s/L);
		else
			ordinate = s * (1 - x/L);
	}
	return ordinate;
}

// bending moment over central support of two-span beam
double CInfluenceLine::LoadEffect2(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = x * (s*s - x*x) / (4*s*s);   
		else
		{
			double y = 2 * s - x;
			ordinate = y * (s*s - y*y) / (4*s*s);
		}
	}
	return ordinate;
}

// left hand shear in simply supported beam
double CInfluenceLine::LoadEffect3(double x)
{
	double L = m_Length;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		ordinate = 1 - x / L;
	}
	return ordinate;
}

// right hand shear in simply-supported beam
double CInfluenceLine::LoadEffect4(double x)
{
	double L = m_Length;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		ordinate = x / L;
	}
	return ordinate;
}

// right hand shear for two-span beam
double CInfluenceLine::LoadEffect5(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = (1/s) * (-x * (s*s - x*x) / (4*s*s) + s - x);
		else
		{
			double y = 2 * s - x;
			ordinate = (1/s) * (-y * (s*s - y*y) / (4*s*s) );
		}
	}
	return ordinate;
}

// left hand shear for two-span beam
double CInfluenceLine::LoadEffect6(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = (1/s) * (-x * (s*s - x*x) / (4*s*s) );
		else
		{
			double y = 2 * s - x;
			ordinate = (1/s) * (-y * (s*s - y*y) / (4*s*s) + s - y);
		}
	}
	return ordinate;
}

// total load on the beam
double CInfluenceLine::LoadEffect7(double x)
{
	if((x < 0.0) || (x > m_Length))
		return 0.0;
	else
		return 1.0;
}
