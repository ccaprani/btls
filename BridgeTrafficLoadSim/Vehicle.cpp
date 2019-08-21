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
	KG_TO_KN	= 9.81 / 1000;

	CASTOR_MAX_AXLES	= 9;
	BEDIT_MAX_AXLES		= 20;
	MON_MAX_AXLES		= 11;
	
	MON_BASE_YEAR = 2010; // to help avoid overflows

	m_TrackWidth = 1.98;
	m_LaneEccentricity = 0.0;

	m_NoAxleGroups = 0;
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
	case 4:
		return createMONVehicle(str);
	default:
		return createCASTORVehicle(str);
	}
}


void CVehicle::createCASTORVehicle(const std::string data)
{
	int sec = 0; int hndt = 0;
	m_Head		= atoi( data.substr(0,4).c_str() );
	m_Day		= atoi( data.substr(4,2).c_str() );
	m_Month		= atoi( data.substr(6,2).c_str() );
	m_Year		= atoi( data.substr(8,2).c_str() );
	m_Hour		= atoi( data.substr(10,2).c_str() );
	m_Min		= atoi( data.substr(12,2).c_str() );
	sec			= atoi( data.substr(14,2).c_str() );
	hndt		= atoi( data.substr(16,2).c_str() );
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
	m_Sec = (double)sec + (double(hndt) / 100);

	setNoAxles(m_NoAxles);
	double W;
	double S;
	int j = 32;
	int NoDigitsAS = 2;			// CASTOR file characteristics
	int NoDigitsAW = 3;

	for (size_t i = 0; i < m_NoAxles; i++)
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
	int sec = 0; int hndt = 0;
	m_Head		= atoi( data.substr(0,4).c_str() );
	m_Day		= atoi( data.substr(4,2).c_str() );
	m_Month		= atoi( data.substr(6,2).c_str() );
	m_Year		= atoi( data.substr(8,2).c_str() );
	m_Hour		= atoi( data.substr(10,2).c_str() );
	m_Min		= atoi( data.substr(12,2).c_str() );
	sec			= atoi( data.substr(14,2).c_str() );
	hndt		= atoi( data.substr(16,2).c_str() );
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
	m_Sec = (double)sec + (double(hndt) / 100);

	setNoAxles(m_NoAxles);
	double W;
	double S;
	int j = 32;					// start reading axle info at index
	int NoDigitsAS = 3;			// BeDIT file characteristics
	int NoDigitsAW = 3;

	for (size_t i = 0; i < m_NoAxles; i++)
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
	int sec = 0; int hndt = 0;
	m_Head		= atoi( data.substr(0,4).c_str() );
	m_Day		= atoi( data.substr(4,2).c_str() );
	m_Month		= atoi( data.substr(6,2).c_str() );
	m_Year		= atoi( data.substr(8,4).c_str() );		// difference to BeDIT here - year is 4 digits
	m_Hour		= atoi( data.substr(12,2).c_str() );
	m_Min		= atoi( data.substr(14,2).c_str() );
	sec			= atoi( data.substr(16,2).c_str() );
	hndt		= atoi( data.substr(18,2).c_str() );
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
	m_Sec = (double)sec + (double(hndt) / 100);

	setNoAxles(m_NoAxles);
	double W;
	double T;
	double S;
	int j = 34;						// start reading axle info at index
	unsigned int NoDigitsAS = 3;	// DITIS file characteristics
	unsigned int NoDigitsAT = 3;
	unsigned int NoDigitsAW = 3;

	for (size_t i = 0; i < m_NoAxles; i++)
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

void CVehicle::createMONVehicle(const std::string data)
{
	m_Head		= atoi(data.substr(0, 9).c_str());
	m_Day		= atoi(data.substr(9, 2).c_str());
	m_Month		= atoi(data.substr(11, 2).c_str());
	m_Year		= atoi(data.substr(13, 4).c_str());
	m_Hour		= atoi(data.substr(17, 2).c_str());
	m_Min		= atoi(data.substr(19, 2).c_str());
	m_Sec		= atoi(data.substr(21, 5).c_str());
	m_NoAxles	= atoi(data.substr(26, 2).c_str());
	m_NoAxleGroups = atoi(data.substr(28, 2).c_str());
	m_GVW		= atoi(data.substr(30, 6).c_str());		// kg
	m_Velocity	= atoi(data.substr(36, 3).c_str());
	m_Length	= atoi(data.substr(39, 5).c_str());
	m_Lane		= atoi(data.substr(44, 1).c_str());
	m_Dir		= atoi(data.substr(45, 1).c_str());
	m_Trns		= atoi(data.substr(46, 4).c_str());

	m_Year -= MON_BASE_YEAR;	// Reduce time of first vehicle
	m_Dir += 1;					// Dir in BeDIT file is zero-based
	m_Length /= 1000;			// Length = length/1000 for mm to meters 
	m_Velocity /= 3.6;			// Vel = vel / 3.6 for km/h to meters/second
	m_GVW *= KG_TO_KN;			// kg to kN
	m_Trns /= 1000;				// mm to meters
	m_Sec /= 1000;				// ms to sec

	setNoAxles(m_NoAxles);
	double W;
	double S;
	int j = 45;					// start reading axle info at index
	int NoDigitsAS = 5;			// MON file characteristics
	int NoDigitsAW = 5;

	for (size_t i = 0; i < m_NoAxles; i++)
	{
		j += NoDigitsAS;
		W = atoi(data.substr(j, NoDigitsAW).c_str());
		W *= KG_TO_KN;	// convert kg to kN
		setAW(i, W);

		j += NoDigitsAW;
		S = atoi(data.substr(j, NoDigitsAS).c_str());
		S /= 1000;		// convert mm to m
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
	case 4:
		return writeMONData();
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
	int sec			= Round(floor(m_Sec));
	int hndt		= Round((m_Sec - sec) * 100);

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(2);	oFile << truncate(m_Year,2); // only use two digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << sec;
	oFile.width(2);	oFile << hndt;
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

	for (size_t i = m_NoAxles; i < CASTOR_MAX_AXLES; i++)
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
	int sec			= Round(floor(m_Sec));
	int hndt		= Round((m_Sec - sec) * 100);

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(2);	oFile << truncate(m_Year,2); // only use two digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << sec;
	oFile.width(2);	oFile << hndt;
	oFile.width(3);	oFile << velocity;
	oFile.width(4);	oFile << grossWeight;
	oFile.width(3);	oFile << length;
	oFile.width(2);	oFile << m_NoAxles;
	oFile.width(1);	oFile << m_Dir-1;	// dir is zero-based in BeDIT files
	oFile.width(1);	oFile << m_Lane;	// local lane no in direction
	oFile.width(3);	oFile << transPos;
	
	size_t len = 3;	// string length
	for (size_t i = 0; i < m_NoAxles; i++)
	{
		oFile.width(len);	oFile << Round(this->getAW(i)/KG100_TO_KN);	// convert from kN to kg/100
		if(i == BEDIT_MAX_AXLES-1) break;	// no extra axle spacing
		oFile.width(len);	oFile << Round(this->getAS(i)*10);		// convert from m to dm
	}

	for (size_t i = m_NoAxles; i < BEDIT_MAX_AXLES; i++)
	{
		oFile.width(len);	oFile << "0";
		if(i == BEDIT_MAX_AXLES-1) break;	// no extra axle spacing
		oFile.width(len);	oFile << "0";
	}

	oFile << std::ends;

	return oFile.str();
}

	/** Prepares a vehicle for printing to a DITIS file	 */

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
	int sec			= Round(floor(m_Sec));
	int hndt		= Round((m_Sec - sec) * 100);

	std::ostringstream oFile;

	oFile.width(4);	oFile << m_Head;
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(4);	oFile << truncate(m_Year,4); // using 4 digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(2);	oFile << sec;
	oFile.width(2);	oFile << hndt;
	oFile.width(3);	oFile << velocity;
	oFile.width(4);	oFile << grossWeight;
	oFile.width(3);	oFile << length;
	oFile.width(2);	oFile << m_NoAxles;
	oFile.width(1);	oFile << m_Dir;		// dir is one-based unlike BeDIT files
	oFile.width(1);	oFile << m_Lane;	// local lane no in direction
	oFile.width(3);	oFile << transPos;
	
	size_t len = 3;	// string length
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

/** Prepares a vehicle for printing to a MON file	 */

std::string CVehicle::writeMONData()
{
	// Reinstate time to min 2010
	// Vel = vel / 3.6 for metres/second to km/h
	// GVW *100 / 0.981 for kN to kg 
	// Length = length*1000 for meters to mm
	// Trns = trns/1000 for metres to mm

	size_t year = m_Year + MON_BASE_YEAR;
	size_t velocity = Round(m_Velocity / 3.6);
	size_t grossWeight = Round(m_GVW * 100 / KG100_TO_KN);
	size_t length = Round(m_Length * 1000);
	size_t transPos = Round(m_Trns * 1000);

	std::ostringstream oFile;

	oFile.width(9);	oFile << m_Head;	// 9 digit ID
	oFile.width(2);	oFile << m_Day;
	oFile.width(2);	oFile << m_Month;
	oFile.width(4);	oFile << year;	// using four digits to represent years
	oFile.width(2);	oFile << m_Hour;
	oFile.width(2);	oFile << m_Min;
	oFile.width(5);	oFile << Round(1000*m_Sec);	// using 5 digits to represent milliseconds
	oFile.width(2);	oFile << m_NoAxles;
	oFile.width(2);	oFile << m_NoAxleGroups;
	oFile.width(6);	oFile << grossWeight;
	oFile.width(3);	oFile << velocity;
	oFile.width(5);	oFile << length;
	oFile.width(1);	oFile << m_Lane;	// local lane no in direction
	oFile.width(1);	oFile << m_Dir - 1;	// dir is zero-based in BeDIT files
	oFile.width(4);	oFile << transPos;

	// Notice ncludes last axle spacing to allow for calculation of overhangs
	size_t len = 5;	// string length
	for (size_t i = 0; i < m_NoAxles; i++)
	{
		oFile.width(len);	oFile << Round(this->getAW(i) / KG_TO_KN);	// convert from kN to kg
		oFile.width(len);	oFile << Round(this->getAS(i) * 1000);		// convert from m to mm
	}
	
	for (size_t i = m_NoAxles; i < MON_MAX_AXLES; i++)
	{
		oFile.width(len);	oFile << "0";
		oFile.width(len);	oFile << "0";
	}

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

void CVehicle::setLane(size_t lane)
{
	m_Lane = lane;
}

void CVehicle::setDirection(size_t dir)
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
	
	m_Sec = temp - m_Min * MINS_PER_HOUR;
	
	m_Day += 1;
	m_Month += 1;
	//m_Year += 99;	// since we can now have year zero, e.g. year 2000 as 00
}

void CVehicle::setAS(size_t i, double s)
{
	m_vAxles[i].Spacing = s;
}

void CVehicle::setAW(size_t i, double w)
{
	m_vAxles[i].Weight = w;
}

void CVehicle::setAT(size_t i, double tw)
{
	m_vAxles[i].TrackWidth = tw;
}

void CVehicle::setNoAxles(size_t axNo)
{
	m_vAxles.clear();
	m_NoAxles = axNo;
	for (size_t i = 0; i < m_NoAxles + 1; i++)
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

void CVehicle::setBridgeLaneNo(size_t BridgeLaneNo)
{
	m_BridgeLaneNo = BridgeLaneNo;	// in global zero-based numbering
}

////////// THE GETS //////////////////

size_t CVehicle::getBridgeLaneNo(void)
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

size_t CVehicle::getLane()
{
	return m_Lane;
}

size_t CVehicle::getDirection()
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
	size_t s_per_hr = SECS_PER_HOUR;
	size_t s_per_day = SECS_PER_HOUR*HOURS_PER_DAY;

	// Note: No. days is double to prevent overflow in the time calculation line
	double no_days = m_Year * (double)(MTS_PER_YR * DAYS_PER_MT) + (m_Month - 1) * (double)(DAYS_PER_MT) + (m_Day-1);
	double time = no_days * s_per_day + m_Hour * s_per_hr + m_Min * SECS_PER_MIN + m_Sec;

	return time;
}

std::string CVehicle::getTimeStr()
{
	std::string sTime = "";
	sTime = to_string(m_Day) + "/" + to_string(m_Month) + "/" + to_string(m_Year) + " ";
	sTime += to_string(m_Hour) + ":" + to_string(m_Min) + ":" + to_string(m_Sec);

	return sTime;
}

size_t CVehicle::getNoAxles()
{
	return m_NoAxles;
}

double CVehicle::getAS(size_t i)
{
	return m_vAxles[i].Spacing;
}

double CVehicle::getAW(size_t i)
{
	return m_vAxles[i].Weight;
}

double CVehicle::getAT(size_t i)
{
	return m_vAxles[i].TrackWidth;
}

double CVehicle::getTrans()
{
	return m_Trns;
}

size_t CVehicle::getHead()
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