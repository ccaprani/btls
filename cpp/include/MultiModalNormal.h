// MultiModalNormal.h: interface for the CMultiModalNormal class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <vector>

class CMultiModalNormal  
{
public:
	void AddMode(double w, double m, double s);
	CMultiModalNormal();
	virtual ~CMultiModalNormal();

	std::size_t getNoModes() const {return m_vModes.size();};

	struct Mode
	{
		double Weight;
		double Mean;
		double StdDev;
	};

	std::vector<Mode> m_vModes;
};
