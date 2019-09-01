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
	void	AddVehicle(CVehicle_up pVeh);
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
	const std::vector<CVehicle_sp> getVehicles(void);

private:
	void	setAxleVector();

	std::vector<CInfluenceLine> m_vInfLine;
	std::vector<CVehicle_sp>	m_vVehicles;
	std::vector<CAxle>			m_vAxles;
	size_t	m_LaneNo;
	double	m_CurTime;
	double	m_Length;
	size_t	m_NoLE;
	double	m_TimeNextVehOff;
	size_t	m_Index;
	double	m_LaneWidth;
};

