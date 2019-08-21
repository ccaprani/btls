#pragma once

#include <vector>
#include "Vehicle.h"
#include "InfluenceLine.h"
#include "Axle.h"

class CBridgeLane
{
public:
	CBridgeLane();
	CBridgeLane(double length);
	~CBridgeLane(void);
	
	bool	operator<(const CBridgeLane& other);
	
	void	Update(double curTime);
	void	setLaneNo(size_t LaneNo);
	size_t	getLaneNo(void);
	void	addLoadEffect(CInfluenceLine IL, double weight);
	void	AddVehicle(CVehicle* pVeh);
	size_t	getIndex(void);
	void	setIndex(size_t indx);
	size_t	purgeVehicles(double curTime);
	double	getLength(void);
	void	setLength(double length);
	double	getTimeNextVehOff(void) const;
	double	setTimeNextVehOff(void);
	double	getLoadEffect(size_t NoLE);
	double	getLeadVehPosition(void);
	size_t	getNoVehs(void);
	std::vector<CVehicle*> getVehicles(void);

private:
	void	setAxleVector();
	//double	doAxleLoop(int nLE);

	std::vector<CInfluenceLine> m_vInfLine;
	std::vector<CVehicle*>		m_vVehicles;
	std::vector<CAxle>			m_vAxles;
	size_t	m_LaneNo;
	double	m_CurTime;
	double	m_Length;
	size_t	m_NoLE;
	double	m_TimeNextVehOff;
	//int		m_NoVehs;
	size_t	m_Index;
	double	m_LaneWidth;
};

