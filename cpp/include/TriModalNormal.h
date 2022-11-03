// TriModalNormal.h: interface for the CTriModalNormal class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRIMODALNORMAL_H__E6522E23_4AF4_460E_B8F8_8AF8D12F482A__INCLUDED_)
#define AFX_TRIMODALNORMAL_H__E6522E23_4AF4_460E_B8F8_8AF8D12F482A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class CTriModalNormal  
{
public:
	void AddMode(double w, double m, double s);
	CTriModalNormal();
	virtual ~CTriModalNormal();

	struct Mode
	{
		double Weight;
		double Mean;
		double StdDev;
	};

	std::vector<Mode> m_vModes;
};

#endif // !defined(AFX_TRIMODALNORMAL_H__E6522E23_4AF4_460E_B8F8_8AF8D12F482A__INCLUDED_)
