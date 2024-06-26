// AxleSpacing.h: interface for the CAxleSpacing class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AXLESPACING_H__429CA222_93DF_47F1_A601_251A90BD7355__INCLUDED_)
#define AFX_AXLESPACING_H__429CA222_93DF_47F1_A601_251A90BD7355__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MultiModalNormal.h"

class CAxleSpacing  
{
public:
	CAxleSpacing();
	virtual ~CAxleSpacing();

	CMultiModalNormal GetSpacingDist(int iTruck, int iSpace);
	void Add2AxleData(std::vector<CMultiModalNormal> vSpace);
	void Add3AxleData(std::vector<CMultiModalNormal> vSpace);
	void Add4AxleData(std::vector<CMultiModalNormal> vSpace);
	void Add5AxleData(std::vector<CMultiModalNormal> vSpace);

private:
	std::vector<CMultiModalNormal> m_v2AxleData;
	std::vector<CMultiModalNormal> m_v3AxleData;
	std::vector<CMultiModalNormal> m_v4AxleData;
	std::vector<CMultiModalNormal> m_v5AxleData;
};

#endif // !defined(AFX_AXLESPACING_H__429CA222_93DF_47F1_A601_251A90BD7355__INCLUDED_)
