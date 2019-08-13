// Vehicle.h: interface for the CVehicle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_)
#define AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include <sstream>
#include <string>

class CVehicle  
{
public:
	CVehicle();
	virtual ~CVehicle();
	
	bool IsCar();
	bool operator<(const CVehicle& x);
	void setHead(int head);
	std::string Write(unsigned int file_type);
	void create(std::string str, unsigned int format);

	// the sets
	void	setTime(double time);
	void	setLength(double length);
	void	setVelocity(double velocity);
	void	setLane(int l);
	void	setDirection(int d);
	void	setGVW(double weight);
	void	setNoAxles(int axNo);
	void	setAW(int i, double w);
	void	setAS(int i, double s);
	void	setAT(int i, double tw);
	void	setTrns(double trans);
	void	setBridgeTimes(double length);
	void	setTimeOnBridge(double length);
	void	setBridgeLaneNo(unsigned int BridgeLaneNo);
	void	setTrackWidth(double tw);
	void	setLaneEccentricity(double e);

	// the gets
	std::string getTimeStr();
	int		getHead();
	double  getTime() const;
	double	getLength();
	double	getVelocity();
	int		getLane();
	int		getDirection();
	double	getGVW();
	int		getNoAxles();
	double	getAW(int i);
	double	getAS(int i);
	double	getAT(int i);
	double	getTrans();
	double	getTimeOnBridge();
	double	getTimeOffBridge();
	bool	IsOnBridge(double atTime);
	unsigned int getBridgeLaneNo(void);
	double	getTrackWidth(void);
	double	getLaneEccentricity(void);

private:
	int Round(double val) {return int(val + 0.5);};
	template <typename T> std::string to_string(T const& value);
	//template<typename T> T from_string(const std::string& s);
	template <typename T> std::string truncate(T const& value, unsigned int digits);

	void createCASTORVehicle(const std::string data);
	void createBEDITVehicle(const std::string data);
	void createDITISVehicle(const std::string data);

	std::string	writeBEDITData();
	std::string	writeCASTORData();
	std::string writeDITISData();

	int		m_Dir;
	int		m_Lane;
	double	m_Velocity;
	int		m_Head;
	int		m_Year;
	int		m_Month;	
	int		m_Day; 	
	int		m_Hour; 
	int		m_Min;
	int		m_Sec; 
	int		m_Hndt;
	double	m_GVW;
	double	m_Trns;
	unsigned int m_NoAxles;
	double	m_Length;
	double	m_TrackWidth; // width
	double	m_TimeOnBridge;
	double	m_TimeOffBridge;
	int		m_BridgeLaneNo;
	double	m_LaneEccentricity;

	struct Axle
	{
		double Weight;
		double Spacing;
		double TrackWidth;
	};
	std::vector<Axle> m_vAxles;

	int		DAYS_PER_MT;
	int		MTS_PER_YR;
	int		HOURS_PER_DAY;
	int		SECS_PER_HOUR;
	int		MINS_PER_HOUR;
	int		SECS_PER_MIN;

	double	KG100_TO_KN;
	int		CASTOR_MAX_AXLES;
	int		BEDIT_MAX_AXLES;

	template<typename T>
	T from_string(const std::string& s)
	{
		// Convert from a string to a T, Type T must support >> operator
		T t;
		std::istringstream ist(s);
		ist >> t;
		return t;
	}

};

#endif // !defined(AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_)
