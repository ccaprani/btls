// Axle.h: interface for the CAxle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_)
#define AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CVehicle; // pre-defined

class CAxle  
{
public:
	CAxle();
	CAxle(int i, double t, double v, double x, double w, double tw, int dirn);
	CAxle(int i, int iAxle, double t, double x, CVehicle* pVeh);
	virtual ~CAxle();
	
	void UpdatePosition(double time);
	
	double m_TimeAtDatum;
	int m_Index;
	double m_Speed;
	double m_Position;
	double m_AxleWeight;
	int m_Dirn;
	double m_TrackWidth;
	double m_TransPos;
	double m_Eccentricity;
	unsigned int m_Lane;

private:
	int m_Sign;
	//CVehicle* m_pVeh;
};

#endif // !defined(AFX_AXLE_H__9EEC5C84_8B3B_4D68_A004_465AAC0FD15A__INCLUDED_)
