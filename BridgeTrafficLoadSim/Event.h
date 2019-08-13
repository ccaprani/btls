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
	CEvent(int);
	CEvent(int, size_t noEffects);
	virtual ~CEvent();
	
	bool operator<(const CEvent& x);

	CEffect&	getMaxEffect(int effNo);
	CEffect&	getMinEffect(int effNo);
	unsigned int	getNoVehicles();
	unsigned int	getNoTrucks();
	int			getID();
	double		getMaxTime();
	double		getMaxEffectTime();
	size_t		getNoEffects();
	double		getStartTime();
	std::string getTimeStr();
	
	void		setStartTime(double StartTime);	
	void		setID(int id);
	void		setMaxEffect(CEffect Eff, int i);
	void		setMinEffect(CEffect Eff, int i);
	void		setNoEffects(size_t noEffects);
	void		setCurEffect(int ce);

	void		writeEffect(int k, std::string file, bool trucks);
	void		writeToFile(std::string file);
	void		AddSingleEffect(CEffect effect);
	
	std::vector<CEffect> m_vMaxEffects;
	std::vector<CEffect> m_vMinEffects;

private:
	void setDefaults();
	template <typename T> std::string to_string(T const& value);

	int		m_CurEffect;
	size_t	m_NoEffects;
	int		m_EventID;
	double	m_StartTime;

	int		DAYS_PER_MT;
	int		MTS_PER_YR;
	int		HOURS_PER_DAY;
	int		SECS_PER_HOUR;
	int		MINS_PER_HOUR;
	int		SECS_PER_MIN;
};

#endif // !defined(AFX_EVENT_H__0B74769F_32EB_4F04_BD76_F27A2F5FF946__INCLUDED_)
