// Event.h: interface for the CEvent class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EVENT_H__0B74769F_32EB_4F04_BD76_F27A2F5FF946__INCLUDED_)
#define AFX_EVENT_H__0B74769F_32EB_4F04_BD76_F27A2F5FF946__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Effect.h"
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>

class CEvent  
{
public:
	CEvent();
	CEvent(size_t);
	CEvent(size_t, size_t noEffects);
	virtual ~CEvent();
	
	bool operator<(const CEvent& x);

	CEffect&	getMaxEffect(size_t effNo);
	CEffect&	getMinEffect(size_t effNo);
	size_t		getNoVehicles();
	size_t		getNoTrucks();
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

#endif // !defined(AFX_EVENT_H__0B74769F_32EB_4F04_BD76_F27A2F5FF946__INCLUDED_)
