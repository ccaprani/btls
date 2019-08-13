#pragma once

#include <vector>

class CInfluenceSurface
{
public:
	CInfluenceSurface(void);
	CInfluenceSurface(std::vector< std::vector<double> > ISmat, std::vector<double> ylanes);
	virtual ~CInfluenceSurface(void);
	double giveOrdinate(double x, double laneEccentricity, unsigned int iLane);
	void setIS(std::vector< std::vector<double> > ISmat);
	void setLanes(std::vector<double> ylanes);
	double getLength();
	double getLaneWidth(unsigned int iLane);

private:
	std::vector< std::vector<double> > m_ISords;
	std::vector<double> m_X; //coordinates;
	std::vector<double> m_Y;
	std::vector<double> m_Ylanes;

	double m_Xmin, m_Xmax, m_Ymin, m_Ymax;
	double m_Length;
	size_t m_NoX, m_NoY;
	size_t m_NoLanes;
};

