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
	void	setLaneNo(int LaneNo);
	int		getLaneNo(void);
	void	addLoadEffect(CInfluenceLine IL, double weight);
	void	AddVehicle(CVehicle* pVeh);
	int		getIndex(void);
	void	setIndex(int indx);
	size_t	purgeVehicles(double curTime);
	double	getLength(void);
	void	setLength(double length);
	double	getTimeNextVehOff(void) const;
	double	setTimeNextVehOff(void);
	double	getLoadEffect(int NoLE);
	double	getLeadVehPosition(void);
	size_t	getNoVehs(void);
	std::vector<CVehicle*> getVehicles(void);

private:
	void	setAxleVector();
	//double	doAxleLoop(int nLE);

	std::vector<CInfluenceLine> m_vInfLine;
	std::vector<CVehicle*>		m_vVehicles;
	std::vector<CAxle>			m_vAxles;
	int		m_LaneNo;
	double	m_CurTime;
	double	m_Length;
	size_t	m_NoLE;
	double	m_TimeNextVehOff;
	//int		m_NoVehs;
	int		m_Index;
	double	m_LaneWidth;
};

