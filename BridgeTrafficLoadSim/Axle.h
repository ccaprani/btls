// Axle.h: interface for the CAxle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_)
#define AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory>
// forward declare
class CVehicle; typedef std::shared_ptr<CVehicle> CVehicle_ptr;

class CAxle  
{
public:
	CAxle();
	CAxle(size_t i, double t, double v, double x, double w, double tw, int dirn);
	CAxle(size_t i, size_t iAxle, double t, double x, CVehicle_ptr pVeh);
	virtual ~CAxle();
	
	void UpdatePosition(double time);
	
	double m_TimeAtDatum;
	size_t m_Index;
	double m_Speed;
	double m_Position;
	double m_AxleWeight;
	size_t m_Dirn;
	double m_TrackWidth;
	double m_TransPos;
	double m_Eccentricity;
	size_t m_Lane;

private:
	int m_Sign;
	//CVehicle_ptr m_pVeh;
};

#endif // !defined(AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_)
