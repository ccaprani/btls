// AxleWeight45.h: interface for the CAxleWeight45 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AXLEWEIGHT45_H__CB51C460_9A57_493D_9649_FA33D6F9805E__INCLUDED_)
#define AFX_AXLEWEIGHT45_H__CB51C460_9A57_493D_9649_FA33D6F9805E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

class CAxleWeight45  
{
public:
	std::vector<double> GetGVWRange(int iTruck, int iRange);
	void AddGVWRange(std::vector<double> data, int iTruck, int iRange);
	CAxleWeight45();
	virtual ~CAxleWeight45();

private:
	struct Dist
	{
		double Mean;
		double StdDev;
	};

	struct GVWRange
	{
		Dist W1;
		Dist W2;
		Dist WT;
	};

	std::vector<GVWRange> m_v4AxleTrucks;
	std::vector<GVWRange> m_v5AxleTrucks;
};

#endif // !defined(AFX_AXLEWEIGHT45_H__CB51C460_9A57_493D_9649_FA33D6F9805E__INCLUDED_)
