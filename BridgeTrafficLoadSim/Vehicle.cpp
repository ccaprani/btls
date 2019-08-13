// Vehicle.cpp: implementation of the CVehicle class.
//
//////////////////////////////////////////////////////////////////////

#include "Vehicle.h"

#include <fstream>
#include <sstream>
#include "ConfigData.h"

extern CConfigData g_ConfigData;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVehicle::CVehicle()
{
	DAYS_PER_MT		= g_ConfigData.Time.DAYS_PER_MT;
	MTS_PER_YR		= g_ConfigData.Time.MTS_PER_YR;
	
	HOURS_PER_DAY	= g_ConfigData.Time.HOURS_PER_DAY;
	SECS_PER_HOUR	= g_ConfigData.Time.SECS_PER_HOUR;
	MINS_PER_HOUR	= g_ConfigData.Time.MINS_PER_HOUR;
	SECS_PER_MIN	= g_ConfigData.Time.SECS_PER_MIN;

	KG100_TO_KN = 0.981;

	CASTOR_MAX_AXLES = 9;
	BEDIT_MAX_AXLES = 20;

	m_TrackWidth = 1.98;
	m_LaneEccentricity = 0.0;
}

CVehicle::~CVehicle()
{

}

//////// CREATE IT ////////////////

void CVehicle::create(std::string str, unsigned int format)
{
	switch(format)
	{
	case 1:
		return createCASTORVehicle(str);
	case 2:
		return createBEDITVehicle(str);
	case 3:
		return createDITISVehicle(str);
	default:
		return createCASTORVehicle(str);
	}
}


void CVehicle::createCASTORVehicle(const std::string data)
{
	m_Head		= atoi( data.substr(0,4).c_str() );
	m_Day		= atoi( data.substr(4,2).c_str() );
	m_Month		= atoi( data.substr(6,2).c_str() );
	m_Year		= atoi( data.substr(8,2).c_str() );
	m_Hour		= atoi( data.substr(10,2).c_str() );
	m_Min		= atoi( data.substr(12,2).c_str() );
	m_Sec		= atoi( data.substr(14,2).c_str() );
	m_Hndt		= atoi( data.substr(16,2).c_str() );
	m_Velocity	= atoi( data.substr(18,3).c_str() );
	m_GVW		= atoi( data.substr(21,4).c_str() );
	m_Length	= atoi( data.substr(25,3).c_str() );
	m_NoAxles	= atoi( data.substr(28,1).c_str() );
	m_Dir		= atoi( data.substr(29,1).c_str() );
	m_Lane		= atoi( data.substr(30,1).c_str() );
	m_Trns		= atoi( data.substr(31,3).c_str() );
	
	m_Length	/= 10;			// Length = length/10 for dm to meters 
	m_Velocity	/= 10;			// Vel = vel / 10 for dm/s to meters/second
	m_GVW		*= KG100_TO_KN;	// GVW * 0.981 for kg/100 to kN
	m_Trns		/= 10;			// Trns = trns/10 for dm to meters
	
	setNoAxles(m_NoAxles);
	double W;
	double S;
	int j = 32;
	int NoDigitsAS = 2;			// CASTOR file characteristics
	int NoDigitsAW = 3;

	for(int i = 0; i < m_NoAxles; i++)
	{
		j += NoDigitsAS;
		W = atoi( data.substr(j,NoDigitsAW).c_str() );
		W *= KG100_TO_KN;	// convert to kN
		setAW(i, W);
		
		j += NoDigitsAW;
		S = atoi( data.substr(j,NoDigitsAS).c_str() );
		S /= 10;		// convert to m
		setAS(i, S);
	}
}


void CVehicle::createBEDITVehicle(const std::string data)
{
	m_Head		= atoi( data.substr(0,4).c_str() );
	m_Day		= atoi( data.substr(4,2).c_str() );
	m_Month		= atoi( data.substr(6,2).c_str() );
	m_Year		= atoi( data.substr(8,2).c_str() );
	m_Hour		= atoi( data.substr(10,2).c_str() );
	m_Min		= atoi( data.substr(12,2).c_str() );
	m_Sec		= atoi( data.substr(14,2).c_str() );
	m_Hndt		= atoi( data.substr(16,2).c_str() );
	m_Velocity	= atoi( data.substr(18,3).c_str() );
	m_GVW		= atoi( data.substr(21,4).c_str() );
	m_Length	= atoi( data.substr(25,3).c_str() );
	m_NoAxles	= atoi( data.substr(28,2).c_str() );
	m_Dir		= atoi( data.substr(30,1).c_str() );
	m_Lane		= atoi( data.substr(31,1).c_str() );
	m_Trns		= atoi( data.substr(32,3).c_str() );
	
	m_Dir += 1;					// Dir in BeDIT file is zero-based
	m_Length	/= 10;			// Length = length/10 for dm to meters 
	m_Velocity	/= 10;			// Vel = vel / 10 for dm/s to meters/second
	m_GVW		*= KG100_TO_KN;	// GVW * 0.981 for kg/100 to kN
	m_Trns		/= 10;			// Trns = trns/10 for dm to meters

	setNoAxles(m_NoAxles);
	double W;
	double S;
	int j = 32;					// start reading axle info at index
	int NoDigitsAS = 3;			// BeDIT file characteristics
	int NoDigitsAW = 3;

	for(int i = 0; i < m_NoAxles; i++)
	{
		j += NoDigitsAS;
		W = atoi( data.substr(j,NoDigitsAW).c_str() );
		W *= KG100_TO_KN;	// convert to kN
		setAW(i, W);
		
		j += NoDigitsAW;
		S = atoi( data.substr(j,NoDigitsAS).c_str() );
		S /= 10;		// convert to m
		setAS(i, S);
	}
}

void CVehicle::createDITISVehicle(const std::string data)
{
	m_Head		= atoi( data.substr(0,4).c_str() );
	m_Day		= atoi( data.substr(4,2).c_str() );
	m_Month		= atoi( data.substr(6,2).c_str() );
	m_Year		= atoi( data.substr(8,4).c_str() );		// difference to BeDIT here - year is 4 digits
	m_Hour		= atoi( data.substr(12,2).c_str() );
	m_Min		= atoi( data.substr(14,2).c_str() );
	m_Sec		= atoi( data.substr(16,2).c_str() );
	m_Hndt		= atoi( data.substr(18,2).c_str() );
	m_Velocity	= atoi( data.substr(20,3).c_str() );
	m_GVW		= atoi( data.substr(23,4).c_str() );
	m_Length	= atoi( data.substr(27,3).c_str() );
	m_NoAxles	= atoi( data.substr(30,2).c_str() );
	m_Dir		= atoi( data.substr(32,1).c_str() );
	m_Lane		= atoi( data.substr(33,1).c_str() );
	m_Trns		= atoi( data.substr(34,3).c_str() );
	
	m_Length	/= 10;			// Length = length/10 for dm to meters 
	m_Velocity	/= 10;			// Vel = vel / 10 for dm/s to meters/second
	m_GVW		*= KG100_TO_KN;	// GVW * 0.981 for kg/100 to kN
	m_Trns		/= 100;			// Trns = trns/100 for cm to meters		// note differrence here to BeDIT

	setNoAxles(m_NoAxles);
	double W;
	double T;
	double S;
	int j = 34;						// start reading axle info at index
	unsigned int NoDigitsAS = 3;	// DITIS file characteristics
	unsigned int NoDigitsAT = 3;
	unsigned int NoDigitsAW = 3;

	for(int i = 0; i < m_NoAxles; i++)
	{
		j += NoDigitsAS;
		W = atoi( data.substr(j,NoDigitsAW).c_str() );
		W *= KG100_TO_KN;	// convert to kN
		setAW(i, W);

		j += NoDigitsAT;
		T = atoi( data.substr(j,NoDigitsAT).c_str() );
		T /= 100;			// convert cm to m
		setAT(i, T);

		j += NoDigitsAW;
		S = atoi( data.substr(j,NoDigitsAS).c_str() );
		S /= 10;			// convert to m
		setAS(i, S);
	}
}

///////// WRITE IT /////////////////

/*
std::string CVehicle::Write()
{
	// CASTOR FORMAT

	// speed from m/s to dm/s
	// Length = length*10 for meters to decimeters
	// GVW / 0.981 for KN to KG/100 
	// Trns = trns/10 for metres to decimetres

	int velocity	= Round(m_Velocity*10);
	int grossWeight = Round(m_GVW/0.981);
	int length		= Round(m_Length*10);
	int transPos	= Round(m_Trns*10);

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(2);	oFile << truncate(m_Year,2); // only use two digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << m_Sec;
	oFile.width(2);	oFile << m_Hndt;
	oFile.width(3);	oFile << velocity;
	oFile.width(4);	oFile << grossWeight;
	oFile.width(3);	oFile << length;
	oFile.width(1);	oFile << m_NoAxles;
	oFile.width(1);	oFile << m_Dir;
	oFile.width(1);	oFile << m_Lane;
	oFile.width(3);	oFile << transPos;
	
	int streamWidth = 3;
	for(int i = 0; i < m_NoAxles; i++)
	{
		oFile.width(streamWidth);	
		oFile << Round(this->getAW(i)/0.981); // kN to kg/100
		streamWidth = 2;

		if(i == 8) 
			break;

		oFile.width(streamWidth);	
		oFile << Round(this->getAS(i)*10);	// m to dm
		streamWidth = 3;
	}

	for(int i = m_NoAxles; i < CASTOR_MAX_AXLES; i++)
	{
		oFile.width(streamWidth);	
		oFile << "0";
		streamWidth = 2;

		if(i == 8) 
			break;

		oFile.width(streamWidth);	
		oFile << "0";
		streamWidth = 3;
	}
	oFile << std::ends;
	return oFile.str();
}
*/

std::string CVehicle::Write(unsigned int file_type)
{
	if(m_Trns < 0.01)	// generated vehicles have 0.0 trans but eccentricity
		m_Trns = 1.80 + m_LaneEccentricity;

	switch(file_type)
	{
	case 1:
		return writeCASTORData();
	case 2:
		return writeBEDITData();
	case 3:
		return writeDITISData();
	default:
		return writeCASTORData();
	}

	// reset trans
	m_Trns = 0.0;
}

	/** Prepares a vehicle for printing to a CASTOR file */

std::string CVehicle::writeCASTORData()
{
	// Length = length*10 for meters to decimeters
	// Vel = vel * 10 for metres/second to decimetres/second
	// GVW / 0.981 for KN to KG/100 
	// Trns = trns/10 for metres to decimetres

	int velocity	= Round(m_Velocity*10);
	int grossWeight = Round(m_GVW/KG100_TO_KN);
	int length		= Round(m_Length*10);
	int transPos	= Round(m_Trns*10);

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(2);	oFile << truncate(m_Year,2); // only use two digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << m_Sec;
	oFile.width(2);	oFile << m_Hndt;
	oFile.width(3);	oFile << velocity;
	oFile.width(4);	oFile << grossWeight;
	oFile.width(3);	oFile << length;
	oFile.width(1);	oFile << m_NoAxles;
	oFile.width(1);	oFile << m_Dir;
	oFile.width(1);	oFile << m_Lane; //getLaneNoInDirection();	// local lane no in direction
	oFile.width(3);	oFile << transPos;
	
	int j = 3;
	for(unsigned int i = 0; i < m_NoAxles; i++)
	{
		oFile.width(j);	oFile << Round(this->getAW(i)/KG100_TO_KN);	// convert from kN to kg/100
		j = 2;

		if(i == 8) break;

		oFile.width(j);	oFile << Round(this->getAS(i)*10);		// convert from m to dm
		j = 3;
	}

	for(unsigned int i = m_NoAxles; i < CASTOR_MAX_AXLES; i++)
	{
		oFile.width(j);	oFile << "0";
		j = 2;

		if(i == 8) break;

		oFile.width(j);	oFile << "0";
		j = 3;
	}

	oFile << std::ends;

	return oFile.str();
}

	/** Prepares a vehicle for printing to a BeDIT file	 */

std::string CVehicle::writeBEDITData()
{
	// Length = length*10 for meters to decimeters
	// Vel = vel * 10 for metres/second to decimetres/second
	// GVW / 0.981 for KN to KG/100 
	// Trns = trns/10 for metres to decimetres

	int velocity	= Round(m_Velocity*10);
	int grossWeight = Round(m_GVW/KG100_TO_KN);
	int length		= Round(m_Length*10);
	int transPos	= Round(m_Trns*10);

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(2);	oFile << truncate(m_Year,2); // only use two digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << m_Sec;
	oFile.width(2);	oFile << m_Hndt;
	oFile.width(3);	oFile << velocity;
	oFile.width(4);	oFile << grossWeight;
	oFile.width(3);	oFile << length;
	oFile.width(2);	oFile << m_NoAxles;
	oFile.width(1);	oFile << m_Dir-1;	// dir is zero-based in BeDIT files
	oFile.width(1);	oFile << m_Lane;	// local lane no in direction
	oFile.width(3);	oFile << transPos;
	
	int len = 3;	// string length
	for(unsigned int i = 0; i < m_NoAxles; i++)
	{
		oFile.width(len);	oFile << Round(this->getAW(i)/KG100_TO_KN);	// convert from kN to kg/100
		if(i == BEDIT_MAX_AXLES-1) break;	// no extra axle spacing
		oFile.width(len);	oFile << Round(this->getAS(i)*10);		// convert from m to dm
	}

	for(unsigned int i = m_NoAxles; i < BEDIT_MAX_AXLES; i++)
	{
		oFile.width(len);	oFile << "0";
		if(i == BEDIT_MAX_AXLES-1) break;	// no extra axle spacing
		oFile.width(len);	oFile << "0";
	}

	oFile << std::ends;

	return oFile.str();
}

	/** Prepares a vehicle for printing to a BeDIT file	 */

std::string CVehicle::writeDITISData()
{
	// Length = length*10 for meters to decimeters
	// Vel = vel * 10 for metres/second to decimetres/second
	// GVW / 0.981 for KN to KG/100 
	// Trns = trns/10 for metres to decimetres

	int velocity	= Round(m_Velocity*10);
	int grossWeight = Round(m_GVW/KG100_TO_KN);
	int length		= Round(m_Length*10);
	int trackwidth	= Round(m_TrackWidth*100); // m to cm
	int transPos	= Round(m_Trns*100);	// m to cm

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(4);	oFile << truncate(m_Year,4); // using 4 digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << m_Sec;
	oFile.width(2);	oFile << m_Hndt;
	oFile.width(3);	oFile << velocity;
	oFile.width(4);	oFile << grossWeight;
	oFile.width(3);	oFile << length;
	oFile.width(2);	oFile << m_NoAxles;
	oFile.width(1);	oFile << m_Dir;		// dir is one-based unlike BeDIT files
	oFile.width(1);	oFile << m_Lane;	// local lane no in direction
	oFile.width(3);	oFile << transPos;
	
	int len = 3;	// string length
	for(unsigned int i = 0; i < m_NoAxles; i++)
	{
		oFile.width(len);	oFile << Round(this->getAW(i)/KG100_TO_KN);	// convert from kN to kg/100
		oFile.width(len);	oFile << Round(this->getAT(i)*100);		// convert from m to cm
		if(i == m_NoAxles-1) 
			break;	// no extra axle spacing
		oFile.width(len);	oFile << Round(this->getAS(i)*10);		// convert from m to dm
	}
/*
	for(unsigned int i = m_NoAxles; i < BEDIT_MAX_AXLES; i++)
	{
		oFile.width(len);	oFile << "0";	// Axle weight
		oFile.width(len);	oFile << "0";	// Axle track width
		if(i == BEDIT_MAX_AXLES-1) break;	// no extra axle spacing
		oFile.width(len);	oFile << "0";
	}
*/
	oFile << std::ends;

	return oFile.str();
}


////////////// THE SETS ////////////////////////////

void CVehicle::setVelocity(double vel)
{
	m_Velocity = vel;
}

void CVehicle::setLength(double length)
{
	m_Length = length;
}

void CVehicle::setLane(int lane)
{
	m_Lane = lane;
}

void CVehicle::setDirection(int dir)
{
	m_Dir = dir;
}

void CVehicle::setGVW(double weight)
{
	m_GVW = weight;
}

void CVehicle::setTime(double time)
{
	double temp = time;	
	
	m_Year	= (int)(temp/(double)(MTS_PER_YR * DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR));
	
	temp	= time - m_Year * (double)(MTS_PER_YR * DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR);	
	m_Month	= (int)(temp/(double)(DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR));
	
	temp	= temp - m_Month * DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR;	
	m_Day	= (int)(temp/(double)(HOURS_PER_DAY * SECS_PER_HOUR));		
	
	temp	= temp - m_Day * HOURS_PER_DAY * SECS_PER_HOUR;			
	m_Hour	= (int)(temp/(double)(SECS_PER_HOUR));
	
	temp	= temp - m_Hour * SECS_PER_HOUR;					
	m_Min	= (int)(temp/(double)(MINS_PER_HOUR));
	
	temp	= temp - m_Min * MINS_PER_HOUR;						
	m_Sec	= (int)(temp);
	
	temp	= temp - m_Sec;							
	m_Hndt	= (int)(100 * temp);
	
	m_Day += 1;
	m_Month += 1;
	//m_Year += 99;	// since we can now have year zero, e.g. year 2000 as 00
}

void CVehicle::setAS(int i, double s)
{
	m_vAxles[i].Spacing = s;
}

void CVehicle::setAW(int i, double w)
{
	m_vAxles[i].Weight = w;
}

void CVehicle::setAT(int i, double tw)
{
	m_vAxles[i].TrackWidth = tw;
}

void CVehicle::setNoAxles(int axNo)
{
	m_vAxles.clear();
	m_NoAxles = axNo;
	for(int i = 0; i < m_NoAxles+1; i++)
	{
		Axle temp;
		temp.Spacing = 0.0;
		temp.Weight = 0.0;
		temp.TrackWidth = m_TrackWidth;
		m_vAxles.push_back(temp);
	}
}

void CVehicle::setTrns(double trans)
{
	m_Trns = trans;
}

void CVehicle::setTrackWidth(double tw)
{
	m_TrackWidth = tw;
}

void CVehicle::setLaneEccentricity(double e)
{
	m_LaneEccentricity = e;
}

void CVehicle::setHead(int head)
{
	m_Head = head;
}

void CVehicle::setBridgeTimes(double BridgeLength)
{
	setTimeOnBridge(BridgeLength);
	m_TimeOffBridge = m_TimeOnBridge + (BridgeLength + m_Length)/(m_Velocity);
}

void CVehicle::setTimeOnBridge(double BridgeLength)
{
/*	// this code is suitable when the time datum is the RHS of the bridge
	if(m_Dir == 1)
		m_TimeOnBridge = getTime();
	else
		m_TimeOnBridge = getTime() - BridgeLength/(m_Velocity);
*/
	// For BTLS, the time datum is the bridge entry point
	m_TimeOnBridge = getTime();
}

void CVehicle::setBridgeLaneNo(unsigned int BridgeLaneNo)
{
	m_BridgeLaneNo = BridgeLaneNo;	// in global zero-based numbering
}

////////// THE GETS //////////////////

unsigned int CVehicle::getBridgeLaneNo(void)
{
	return m_BridgeLaneNo;
}

double CVehicle::getTimeOffBridge()
{
	return m_TimeOffBridge;
}

double CVehicle::getTimeOnBridge()
{
	return m_TimeOnBridge;
}

double CVehicle::getVelocity()
{
	return m_Velocity;
}

double CVehicle::getLength()
{
	return m_Length;
}

int CVehicle::getLane()
{
	return m_Lane;
}

int CVehicle::getDirection()
{
	return m_Dir;
}

double CVehicle::getGVW()
{
	return m_GVW;
}

double CVehicle::getTrackWidth()
{
	return m_TrackWidth;
}

double CVehicle::getLaneEccentricity()
{
	return m_LaneEccentricity;
}

double CVehicle::getTime() const
{
	// working off a 10 month year & a 25 day month
	int s_per_hr = SECS_PER_HOUR;
	int s_per_day = SECS_PER_HOUR*HOURS_PER_DAY;

	// Note: No. days is double to prevent overflow in the time calculation line
	double no_days = m_Year * MTS_PER_YR * DAYS_PER_MT + (m_Month - 1) * DAYS_PER_MT + (m_Day-1);
	double time = no_days * s_per_day + m_Hour * s_per_hr + m_Min * SECS_PER_MIN + m_Sec + (double)m_Hndt/100;

	return time;
}

std::string CVehicle::getTimeStr()
{
	std::string sTime = "";
	sTime = to_string(m_Day) + "/" + to_string(m_Month) + "/" + to_string(m_Year) + " ";
	sTime += to_string(m_Hour) + ":" + to_string(m_Min) + ":" + to_string(m_Sec);

	return sTime;
}

int CVehicle::getNoAxles()
{
	return m_NoAxles;
}

double CVehicle::getAS(int i)
{
	return m_vAxles[i].Spacing;
}

double CVehicle::getAW(int i)
{
	return m_vAxles[i].Weight;
}

double CVehicle::getAT(int i)
{
	return m_vAxles[i].TrackWidth;
}

double CVehicle::getTrans()
{
	return m_Trns;
}

int CVehicle::getHead()
{
	return m_Head;
}

bool CVehicle::IsCar()
{
	return (m_NoAxles == 2 && m_GVW < 34.334);	// MAGIC NUMBER - CAR WEIGHT THRESHOLD
}

bool CVehicle::IsOnBridge(double atTime)
{
	// <= as it's coming on and will be on
	// < as it's leaving: <= would mean about to leave
	if( getTimeOnBridge() <= atTime && atTime < getTimeOffBridge() )
		return true;
	else
		return false;
}

template <typename T>
std::string CVehicle::to_string(T const& value)
{
	std::stringstream sstr;
    sstr << value;
    return sstr.str();
}

template <typename T>
std::string CVehicle::truncate(T const& value, unsigned int digits)
{
	std::stringstream sstr;
    sstr << value;
	std::string str = sstr.str();
	size_t length = str.length();
	if(length <= digits)
		return str;
	else
		return str.substr(length-digits, length);
}

bool CVehicle::operator<(const CVehicle& x)
{
	double a = this->getTime();
	double b = x.getTime();
	return a < b;
}