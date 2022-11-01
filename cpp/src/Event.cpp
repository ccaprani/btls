// Event.cpp: implementation of the CEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "Event.h"
#include <sstream>
#include "ConfigData.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CEvent::CEvent(size_t fileFormat) : FILE_FORMAT(fileFormat)
{
	setDefaults();
}

CEvent::CEvent(size_t fileFormat, size_t ID) : FILE_FORMAT(fileFormat)
{
	setDefaults();
	m_EventID = ID;
}

CEvent::CEvent(size_t fileFormat, size_t ID, size_t noEffects) : FILE_FORMAT(fileFormat)
{
	setDefaults();
	m_EventID = ID;
	m_NoEffects = noEffects;
	setNoEffects(noEffects);
}

CEvent::~CEvent()
{

}

bool CEvent::operator<(const CEvent& x)
{
	double a = this->m_vMaxEffects[m_CurEffect].getValue();
	double b = x.m_vMaxEffects[m_CurEffect].getValue(); 
	return a < b;
}

void CEvent::setDefaults()
{
	// We're not wrapping the time constants, so just use the internal values
	DAYS_PER_MT		= CConfigData::get().Time.DAYS_PER_MT;
	MTS_PER_YR		= CConfigData::get().Time.MTS_PER_YR;
	
	HOURS_PER_DAY	= CConfigData::get().Time.HOURS_PER_DAY;
	SECS_PER_HOUR	= CConfigData::get().Time.SECS_PER_HOUR;
	MINS_PER_HOUR	= CConfigData::get().Time.MINS_PER_HOUR;
	SECS_PER_MIN	= CConfigData::get().Time.SECS_PER_MIN;

	m_EventID = 0;
	reset();
}

void CEvent::reset() 
{
	m_CurEffect = 0;
	m_StartTime = 0.0;	
	m_EventID = 0;
	m_NoEffects = 0;	

	m_vMaxEffects.clear();
	m_vMinEffects.clear();
}

//////// THE SETS /////////////

void CEvent::setMaxEffect(CEffect Eff, size_t i)
{
	m_vMaxEffects[i] = Eff;
}

void CEvent::setMinEffect(CEffect Eff, size_t i)
{
	m_vMinEffects[i] = Eff;
}

void CEvent::setID(size_t id)
{
	m_EventID = id;
}

void CEvent::setNoEffects(size_t noEffects)
{
	CEffect maxEff;
	CEffect minEff(1e90,0,0);	// MAGIC NUMBER - very large to start minimum off
	m_NoEffects = noEffects;
	for (size_t i = 0; i < noEffects; i++)
	{
		m_vMaxEffects.push_back(maxEff);
		m_vMinEffects.push_back(minEff);
	}
}

void CEvent::setCurEffect(size_t ce)
{
	m_CurEffect = ce;
}

void CEvent::setStartTime(double StartTime)
{
	m_StartTime = StartTime;
}

////////// THE GETS /////////////////

size_t CEvent::getNoVehicles()
{
	return m_vMaxEffects[0].m_NoVehicles;
}

size_t CEvent::getNoTrucks() const
{
	size_t n = 0;
	for (size_t i = 0; i < m_vMaxEffects[0].m_NoVehicles; i++)
		if( !m_vMaxEffects[0].giveVehicle(i).IsCar() )
			n++;
	return n;
}

double CEvent::getStartTime()
{
	return m_StartTime;
}

size_t CEvent::getID()
{
	return m_EventID;
}

size_t	CEvent::getNoEffects()
{
	return m_NoEffects;
}

CEffect& CEvent::getMaxEffect(size_t effNo)
{
	if(effNo < m_NoEffects)
		return m_vMaxEffects[effNo];
	else
	{
		std::cout << "***Error: cannot return effect no. " << effNo << endl;
		return m_vMaxEffects[0];
	}
}

CEffect& CEvent::getMinEffect(size_t effNo)
{
	if(effNo < m_NoEffects)
		return m_vMinEffects[effNo];
	else
	{
		std::cout << "***Error: cannot return effect no. " << effNo << endl;
		return m_vMinEffects[0];
	}
}

double CEvent::getMaxEffectTime()
{
	double time = 0.0;
	if(m_vMaxEffects.size() > 0) 
		time = m_vMaxEffects[m_CurEffect].getTime();
	return time;
}

double CEvent::getMaxTime()
{
	double time = 0.0;
	for(unsigned int i = 0; i < m_vMaxEffects.size(); i++)
	{
		double tempT = m_vMaxEffects[i].getTime();
		if(time < tempT)
			time = tempT;
	}
	return time;
}

void CEvent::AddSingleEffect(CEffect eff)
{
	m_NoEffects++;
	m_vMaxEffects.push_back(eff);
}

void CEvent::writeToFile(string file)
{
	// ofstream constructor opens file for appending data
	const char* pFile; pFile = file.c_str();
	ofstream outFile( pFile, ios::app );
    
	// check to see if file was created
	if( !outFile )
	{
		std::cerr << "Event file could not be opened" << endl;
		exit( 1 );
	}

	outFile << m_EventID << endl;
//	outFile.close();

	for (size_t k = 0; k < m_NoEffects; k++)
		writeEffect(k, file, 1);

	outFile.close();
}

void CEvent::writeEffect(size_t k, string file, bool trucks)
{
	// ofstream constructor opens file fo appending data
	ofstream outFile( file.c_str(), ios::app );
    
	// check to see if file was created
	if( !outFile ){
		::cerr << "File could not be opened" << endl;
		exit( 1 );
	}

	double val, time, dist;			size_t nTks;
	ostringstream oStr;

	val = m_vMaxEffects[k].getValue();	
	time = m_vMaxEffects[k].getTime();
	dist = m_vMaxEffects[k].getPosition();

	oStr.width(2);		oStr << k+1;		
	oStr.width(10);		oStr << std::fixed << std::setprecision(1) << val;
	oStr.width(15);		oStr << std::fixed << std::setprecision(1) << time;	
	oStr.width(10);		oStr << std::fixed << std::setprecision(2) << dist;
	oStr.width(4);		nTks = m_vMaxEffects[k].m_NoVehicles;	
	oStr << nTks;// << ends;
	// oStr << ends;
	
	outFile << oStr.str() << endl;
	if(trucks)
	{
		m_vMaxEffects[k].sortVehicles();
		for (size_t j = 0; j < nTks; j++)
			outFile << m_vMaxEffects[k].giveVehicle(j).Write(FILE_FORMAT) << '\n';
	}
//	outFile.close();
}

std::string CEvent::getTimeStr()
{
	double temp = m_StartTime;	
	
	int Year	= (int)(temp/(double)(MTS_PER_YR * DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR));
	
	temp		= temp - Year * (double)(MTS_PER_YR * DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR);	
	int Month	= (int)(temp/(double)(DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR));
	
	temp		= temp - Month * DAYS_PER_MT * HOURS_PER_DAY * SECS_PER_HOUR;	
	int Day		= (int)(temp/(double)(HOURS_PER_DAY * SECS_PER_HOUR));		
	
	temp		= temp - Day * HOURS_PER_DAY * SECS_PER_HOUR;			
	int Hour	= (int)(temp/(double)(SECS_PER_HOUR));
	
	temp		= temp - Hour * SECS_PER_HOUR;					
	int Min		= (int)(temp/(double)(MINS_PER_HOUR));
	
	temp		= temp - Min * MINS_PER_HOUR;						
	int Sec		= (int)(temp);

	Day += 1;
	Month += 1;

	std::string sTime = "";
	sTime = to_string(Day) + "/" + to_string(Month) + "/" + to_string(Year) + " ";
	sTime += to_string(Hour) + ":" + to_string(Min) + ":" + to_string(Sec);

	return sTime;
}

template <typename T>
std::string CEvent::to_string(T const& value)
{
	std::stringstream sstr;
    sstr << value;
    return sstr.str();
}