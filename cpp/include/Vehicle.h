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
#include <memory>
#include <math.h>
#include "VehicleClassification.h"

class CVehicle  
{
public:
	CVehicle();
	virtual ~CVehicle();
	
	bool IsCar();
	bool operator<(const CVehicle& x);
	void setHead(int head);
	std::string Write(size_t file_type);
	void create(std::string str, size_t format);

	// the sets
	void	setTime(double time);
	void	setLength(double length);
	void	setVelocity(double velocity);
	void	setLocalLane(size_t l);
	void	setGlobalLane(size_t l, size_t nRoadLanes);
	void	setDirection(size_t d);
	void	setGVW(double weight);
	void	setNoAxles(size_t axNo);
	void	setAW(size_t i, double w);
	void	setAS(size_t i, double s);
	void	setAT(size_t i, double tw);
	void	setTrns(double trans);
	void	setBridgeTimes(double BridgeLength);
	void	setTimeOnBridge(double BridgeLength);
	void	setBridgeLaneNo(size_t BridgeLaneNo);
	void	setTrackWidth(double tw);
	void	setLaneEccentricity(double e);
	void	setClass(Classification cl);

	// the gets
	std::string getTimeStr();
	size_t	getHead();
	double  getTime() const;
	double	getLength();
	double	getVelocity();
	size_t	getLocalLane();
	size_t	getGlobalLane(size_t nRoadLanes);
	size_t	getDirection();
	double	getGVW();
	size_t	getNoAxles();
	double	getAW(size_t i);
	double	getAS(size_t i);
	double	getAT(size_t i);
	double	getTrans();
	double	getTimeOnBridge();
	double	getTimeOffBridge();
	bool	IsOnBridge(double atTime);
	size_t	getBridgeLaneNo(void);
	double	getTrackWidth(void);
	double	getLaneEccentricity(void);
	Classification getClass();

private:
	int Round(double val) {return int(val + 0.5);};
	template <typename T> std::string to_string(T const& value);
	//template<typename T> T from_string(const std::string& s);
	template <typename T> std::string truncate(T const& value, unsigned int digits);

	void createCASTORVehicle(const std::string data);
	void createBEDITVehicle(const std::string data);
	void createDITISVehicle(const std::string data);
	void createMONVehicle(const std::string data);

	std::string	writeBEDITData();
	std::string	writeCASTORData();
	std::string writeDITISData();
	std::string writeMONData();

	Classification m_Class;

	size_t	m_Dir;
	size_t	m_Lane;
	double	m_Velocity;
	size_t	m_Head;
	size_t	m_Year;
	size_t	m_Month;
	size_t	m_Day;
	size_t	m_Hour;
	size_t	m_Min;
	double	m_Sec;
	double	m_GVW;
	double	m_Trns;
	size_t	m_NoAxles;
	size_t	m_NoAxleGroups;
	double	m_Length;
	double	m_TrackWidth; // width
	double	m_TimeOnBridge;
	double	m_TimeOffBridge;
	size_t	m_BridgeLaneNo;
	double	m_LaneEccentricity;

	struct Axle
	{
		double Weight;
		double Spacing;
		double TrackWidth;
	};
	std::vector<Axle> m_vAxles;

	size_t	DAYS_PER_MT;
	size_t	MTS_PER_YR;
	size_t	HOURS_PER_DAY;
	size_t	SECS_PER_HOUR;
	size_t	MINS_PER_HOUR;
	size_t	SECS_PER_MIN;

	double	KG100_TO_KN;
	double	KG_TO_KN;
	size_t	CASTOR_MAX_AXLES;
	size_t	BEDIT_MAX_AXLES;
	size_t	MON_MAX_AXLES;
	size_t	MON_BASE_YEAR;

	//enum eFileFormat
	//{
	//	eCASTOR,
	//	eBEDIT,
	//	eDITIS,
	//	eMON
	//} m_FileFormat;

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
typedef std::unique_ptr<CVehicle> CVehicle_up;
typedef std::weak_ptr<CVehicle> CVehicle_wp;
typedef std::shared_ptr<CVehicle> CVehicle_sp;

#endif // !defined(AFX_VEHICLE_H__028A909A_9588_4305_9A3E_D255BD8D332A__INCLUDED_)
