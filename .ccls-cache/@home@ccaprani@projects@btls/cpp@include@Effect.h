// Effect.h: interface for the CEffect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EFFECT_H__0EFA9024_74FF_4E82_BF7C_D5F28B09284B__INCLUDED_)
#define AFX_EFFECT_H__0EFA9024_74FF_4E82_BF7C_D5F28B09284B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Vehicle.h"
#include <vector>
#include <iostream>
#include <algorithm>

class CEffect  
{
public:
	CEffect();
	CEffect(double value, double time, double distance);
	virtual ~CEffect();
	bool operator<(const CEffect& x);	

	double getPosition();
	double getTime();
	double getValue() const;
	void setTime(double time);
	void setValue(double time);
	void setPosition(double time);
	CVehicle giveVehicle(size_t i) const;
	void AddVehicle(CVehicle& Vehicle);
	void AddVehicles(std::vector<CVehicle> vVeh);
	void sortVehicles();

	size_t m_NoVehicles;
	size_t m_IndEvent;
	size_t m_IndEff;

private:
	double m_Time;
	double m_Val;
	double m_Dist;
	std::vector<CVehicle> m_vVehicles;
};

#endif // !defined(AFX_EFFECT_H__0EFA9024_74FF_4E82_BF7C_D5F28B09284B__INCLUDED_)
