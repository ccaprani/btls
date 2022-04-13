#include "InfluenceSurface.h"


CInfluenceSurface::CInfluenceSurface(void)
{
	m_Length = 0.0;
	m_Xmin = 0.0;
	m_Xmax = 0.0;
	m_Ymin = 0.0;
	m_Ymax = 0.0;
	m_NoX = 0;
	m_NoY = 0;
}

CInfluenceSurface::CInfluenceSurface(std::vector< std::vector<double> > ISmat, std::vector<double> ylanes)
{
	m_Length = 0.0;
	m_Xmin = 0.0;
	m_Xmax = 0.0;
	m_Ymin = 0.0;
	m_Ymax = 0.0;
	m_NoX = 0;
	m_NoY = 0;
	setIS(ISmat);
	setLanes(ylanes);
}

CInfluenceSurface::~CInfluenceSurface(void)
{

}

void CInfluenceSurface::setLanes(std::vector<double> ylanes)
{
	m_Ylanes = ylanes;	// the y-coords of the near-side edges of the lanes
	m_NoLanes = m_Ylanes.size();
}

double CInfluenceSurface::getLaneWidth(size_t iLane)
{
	double width = m_Ylanes.at(iLane+1)-m_Ylanes.at(iLane);
	return width;
}

void CInfluenceSurface::setIS(std::vector< std::vector<double> > ISmat)
{
	size_t nr = ISmat.size();
	size_t nc = ISmat.at(0).size();

	// get x-coords
	for(size_t i = 1; i < nr; i++)
		m_X.push_back(ISmat.at(i).at(0));

	// get y-coords
	for(size_t j = 1; j < nc; j++)
		m_Y.push_back(ISmat.at(0).at(j));

	// Fill IS ordinates
	for(size_t i = 1; i < nr; i++)
	{
		std::vector<double> vec;
		for(size_t j = 1; j < nc; j++)
			vec.push_back( ISmat.at(i).at(j) );
		m_ISords.push_back(vec);
	}
	
	m_NoX = nr - 1;
	m_NoY = nc - 1;
	m_Xmin = m_X.at(0);
	m_Xmax = m_X.at(m_NoX-1);
	m_Ymin = m_Y.at(0);
	m_Ymax = m_Y.at(m_NoY-1);
	
	m_Length = m_Xmax - m_Xmin;
}

double CInfluenceSurface::giveOrdinate(double x, double laneEccentricity, size_t iLane)
{
	// x is the position along the length of the bridge
	// ylocal is the transverse position within lane number iLane

	//if(iLane > m_NoLanes) return 0.0; // better off to crash here though?
	double yLaneCentre = (m_Ylanes.at(iLane)+m_Ylanes.at(iLane+1))/2;
	double y = yLaneCentre + laneEccentricity; // y is now global wrt influence surface

	// check we are within the bounds of the surface
	double buffer = 0.0;	// tiny additional for end points
	if(x < m_Xmin - buffer || x > m_Xmax + buffer)	
		return 0.0;
	if(y < m_Ymin - buffer || y > m_Ymax + buffer)
		return 0.0;

	size_t iX = 1;
	size_t iY = 1;
	
	// find the indices
	while(x >= m_X.at(iX) && iX < m_NoX-1) iX++;
	while(y >= m_Y.at(iY) && iY < m_NoY-1) iY++;

	// Are we right at the end or edge of the IL?
	if(x >= m_Xmax - buffer && x <= m_Xmax + buffer) iX = m_NoX-1;
	if(y >= m_Ymax - buffer && y <= m_Ymax + buffer) iY = m_NoY-1;

	double deltaX = m_X.at(iX) - m_X.at(iX-1);
	double deltaY = m_Y.at(iY) - m_Y.at(iY-1);

	double xsi1 = m_ISords.at(iX-1).at(iY);
	double xsi2 = m_ISords.at(iX).at(iY);
	double xsi3 = m_ISords.at(iX-1).at(iY-1);
	double xsi4 = m_ISords.at(iX).at(iY-1);

	// interp along x-direction first
	double xsiA = xsi1 + (x-m_X.at(iX-1))/deltaX*(xsi2-xsi1);	// high y value
	double xsiB = xsi3 + (x-m_X.at(iX-1))/deltaX*(xsi4-xsi3);	// low y value

	// and finally along y-direction
	double xsi = xsiB + (y-m_Y.at(iY-1))/deltaY*(xsiA-xsiB);
	
	return xsi;
}

double CInfluenceSurface::getLength()
{
	return m_Length;
}