#pragma once

#include <vector>
#include "Axle.h"
#include "InfluenceSurface.h"

class CInfluenceLine
{
public:
	CInfluenceLine(void);
	~CInfluenceLine(void);
	// get the ordinate at the position x
	double getOrdinate(double x);
	size_t getNoPoints(void);
	double getLoadEffect(std::vector<CAxle>& vAxle);
	size_t getType() { return m_Type; };
	CInfluenceSurface* getIS();

	// set the influence line no.
	void setIL(size_t nIL, double length);
	void setIL(std::vector<double> vDis,std::vector<double> vOrd);
	void setIL(CInfluenceSurface IS);
	double getLength(void);
	void setIndex(int n);
	int getIndex(void);
	void setWeight(double weight);

private:
	double getAxleLoadEffect(CAxle& axle);
	double getEquationOrdinate(double x);
	double getDiscreteOrdinate(double x);
	double LoadEffect1(double x);
	double LoadEffect2(double x);
	double LoadEffect3(double x);
	double LoadEffect4(double x);
	double LoadEffect5(double x);
	double LoadEffect6(double x);
	double LoadEffect7(double x);

	size_t m_Type;			// 1 - expression, 2 - discrete, 3 - Surface
	size_t m_ILFunctionNo;	// If Type = 1, then the IL function no.
	size_t m_Index;			// just a reference no.
	double m_Length;
	size_t m_NoPoints;
	double m_Weight;
	std::vector<double> m_vDistance;
	std::vector<double> m_vOrdinate;
	
	CInfluenceSurface m_IS;

	// pointer to correct load effect function to avoid decision each call
	typedef double (CInfluenceLine::*LEfptr)(double x);
	std::vector<LEfptr> m_vLEfptr;
	LEfptr m_LEfptr;	
};

