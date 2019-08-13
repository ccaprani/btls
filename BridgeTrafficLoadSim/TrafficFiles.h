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
#include "LaneFlow.h"

class CTrafficFiles
{
public:	
	CTrafficFiles();//int siteW);
	virtual ~CTrafficFiles();
	
	CTrafficData GetTrafficData();
	void ReadAll(int HWmodel);
	void ReadPhysical();
	void ReadFile_NHM();
	std::vector<CLaneFlow> ReadLaneFlow(std::string file);

private:
	CCSVParse m_CSV;	
	CTrafficData m_TD;
	
	//int siteW;
	//int siteQ;
	std::string m_Path;
	//std::string m_Root;
	//std::string m_PathW;
	//std::string m_PathQ;
	//std::string m_FolderW;
	//std::string m_FolderQ;

	void ReadFile_AW23();
	void ReadFile_AW45();
	void ReadFile_AS();
	void ReadFile_GVW();
	void ReadFile_ATW();
	//void ReadFile_CP();
	//void ReadFile_Q();
	
};

#endif // !defined(AFX_TrafficFiles_H__25102AA8_BD81_4AA9_815E_81EAE2C49E0C__INCLUDED_)
