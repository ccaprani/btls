// Bridge.h: interface for the CBridge class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BRIDGE_H__08978135_0EE8_43D7_83DC_10A50D3EA1F3__INCLUDED_)
#define AFX_BRIDGE_H__08978135_0EE8_43D7_83DC_10A50D3EA1F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include <boost/thread.hpp>
//#include <boost/ref.hpp>
#include "BridgeLane.h"
#include "EventManager.h"
#include "Vehicle.h"
#include "CalcEffect.h"
#include <memory>

class CBridge;  typedef std::shared_ptr<CBridge> CBridge_sp;

class CBridge  
{
public:
	CBridge();
	CBridge(double length, double calcTimeStep, int n, double curTime);
	virtual ~CBridge();

	void setCurrentSimTime(double curTime);
	void AddVehicle(const CVehicle_sp& pVeh);
	void setCalcTimeStep(double calcTimeStep);
	void setLength(double length);
	void Update(double NextArrivalTime, double curTime);
	void setThresholds(std::vector<double> vThresholds);

    //void UpdateMT(double NextArrivalTime, double curTime);
    //void join();

	void Finish();
	size_t getIndex(void);
	void setIndex(size_t index);
	double getLength(void);
	void InitializeLanes(size_t NoLanes);
	void setNoLoadEffects(size_t nLE);
	size_t getNoLoadEffects(void);
	void InitializeDataMgr(double SimStartTime);
	CBridgeLane& getBridgeLane(size_t iLane);
	size_t getNoLanes();

private:
	bool	lane_compare(const CBridgeLane* pL1, const CBridgeLane* pL2);
	double	TimeNextVehOffBridge();
	double	EventEndTime();	
	const std::vector<CVehicle_sp> AssembleVehicles(void);
	
	CEventManager				m_EventMgr;
	CCalcEffect					m_CalcEff;
	std::vector<double>			m_vEffectValues;
	std::vector<double>			m_vThresholds;
	std::vector<CBridgeLane>	m_vLanes;

	double	m_CurTime;
	double	m_CalcTimeStep;
	double	m_Length;
	
	size_t	NO_LANES_DIR1;
	size_t	NO_DIRS;
	size_t	NO_LANES;
	size_t	m_Index;
	size_t	m_NoLanes;
	size_t	m_NoLoadEffects;
	size_t	m_NoVehs;

//	boost::thread m_Thread;	
};

#endif // !defined(AFX_BRIDGE_H__08978135_0EE8_43D7_83DC_10A50D3EA1F3__INCLUDED_)
