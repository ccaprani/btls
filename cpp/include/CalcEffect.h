// CalcEffect.h: interface for the CCalcEffect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CALCEFFECT_H__3FD6FAB1_0A36_43E9_9240_1CBF454DABF4__INCLUDED_)
#define AFX_CALCEFFECT_H__3FD6FAB1_0A36_43E9_9240_1CBF454DABF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCalcEffect  
{
public:
	void setSpan(double span);
	CCalcEffect();
	CCalcEffect(double span);
	virtual ~CCalcEffect();

	double giveEffect(int j, double X, double AW, int lane);

private:
	double m_span;
	double give_effect1(double x);	
	double give_effect2(double x);	
	double give_effect3(double x);
	double give_effect4(double x);	
	double give_effect5(double x);	
	double give_effect6(double x);
	double give_effect7(double x);
	double give_effectA1(double x);
	double give_effectA2(double x);
	double give_effectB1(double x);
	double give_effectB2(double x);
	double give_effectC1(double x);
	double give_effectC2(double x);
	double give_Beam1(double x);
	double give_Beam2(double x);
	double give_Beam3(double x);
	double give_Beam4(double x);
	double give_Beam5(double x);
	double give_effect16(double x);
};

#endif // !defined(AFX_CALCEFFECT_H__3FD6FAB1_0A36_43E9_9240_1CBF454DABF4__INCLUDED_)
