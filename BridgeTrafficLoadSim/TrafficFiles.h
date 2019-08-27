// TrafficFiles.h: doubleerface for the CTrafficFiles class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TrafficFiles_H__25102AA8_BD81_4AA9_815E_81EAE2C49E0C__INCLUDED_)
#define AFX_TrafficFiles_H__25102AA8_BD81_4AA9_815E_81EAE2C49E0C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "CSVParse.h"
#include "TriModalNormal.h"
#include "TrafficData.h"

class CTrafficFiles
{
public:	
	CTrafficFiles();
	virtual ~CTrafficFiles();
	
	CTrafficData GetTrafficData();
	void ReadPhysical();

private:
	CCSVParse m_CSV;	
	CTrafficData m_TD;
	
	std::string m_Path;

	void ReadFile_AW23();
	void ReadFile_AW45();
	void ReadFile_AS();
	void ReadFile_GVW();
	void ReadFile_ATW();	
};

#endif // !defined(AFX_TrafficFiles_H__25102AA8_BD81_4AA9_815E_81EAE2C49E0C__INCLUDED_)
