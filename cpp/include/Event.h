// Event.h: interface for the CEvent class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "Effect.h"
#include "ConfigData.h"

class CEvent  
{
public:
	CEvent(size_t file_format);
	CEvent(size_t file_format, size_t ID);
	CEvent(size_t file_format, size_t ID, size_t noEffects);

	virtual ~CEvent();
	
	bool operator<(const CEvent& x);

	void reset();

	CEffect&	getMaxEffect(size_t effNo);
	CEffect&	getMinEffect(size_t effNo);
	size_t		getNoVehicles();
	size_t		getNoTrucks() const;
	size_t		getID();
	double		getMaxTime();
	double		getMaxEffectTime();
	size_t		getNoEffects();
	double		getStartTime();
	std::string getTimeStr();
	
	void		setStartTime(double StartTime);	
	void		setID(size_t id);
	void		setMaxEffect(CEffect Eff, size_t i);
	void		setMinEffect(CEffect Eff, size_t i);
	void		setNoEffects(size_t noEffects);
	void		setCurEffect(size_t ce);

	void		writeEffect(size_t k, std::string file, bool trucks);
	void		writeToFile(std::string file);
	void		AddSingleEffect(CEffect effect);
	
	std::vector<CEffect> m_vMaxEffects;
	std::vector<CEffect> m_vMinEffects;

private:
	void setDefaults();
	template <typename T> std::string to_string(T const& value);

	size_t	m_CurEffect;
	size_t	m_NoEffects;
	size_t	m_EventID;
	double	m_StartTime;

	size_t	DAYS_PER_MT;
	size_t	MTS_PER_YR;
	size_t	HOURS_PER_DAY;
	size_t	SECS_PER_HOUR;
	size_t	MINS_PER_HOUR;
	size_t	SECS_PER_MIN;
	
	size_t	FILE_FORMAT;
};