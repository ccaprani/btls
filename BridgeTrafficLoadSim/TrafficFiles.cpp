// TrafficFiles.cpp: implementation of the CTrafficFiles class.
//
//////////////////////////////////////////////////////////////////////

#include "TrafficFiles.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;
using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTrafficFiles::CTrafficFiles()//int siteW)
{
/*	int siteQ = 2;	// This is now superseeded by the lane definition file

	int QW[2];
	std::string m_Folder;

	QW[0] = siteQ;
	QW[1] = siteW;

	for(int i = 0; i<2; i++)
	{
		switch(QW[i])
		{
		case 1:
			{ m_Folder = "Angers\\"; break;}
		case 2:
			{ m_Folder = "Auxerre\\"; break;}
		case 3:
			{ m_Folder = "A196\\"; break;}
		case 4:
			{ m_Folder = "B224\\"; break;}
		case 5:
			{ m_Folder = "A296\\"; break;}
		case 6:
			{ m_Folder = "Samaris\\D1\\"; break;}
		case 7:
			{ m_Folder = "Samaris\\D2\\"; break;}
		case 8:
			{ m_Folder = "Samaris\\D3\\"; break;}
		case 9:
			{ m_Folder = "Samaris\\S1\\"; break;}
		case 10:
			{ m_Folder = "Samaris\\S2\\"; break;}
		case 11:
			{ m_Folder = "Samaris\\S3\\"; break;}
		case 12:
			{ m_Folder = "Samaris\\D\\"; break;}
		case 13:
			{ m_Folder = "Samaris\\S\\"; break;}
		}

		if(i == 0)
			m_FolderW = m_Folder;
		else
			m_FolderQ = m_Folder;
	}

	m_Root = "C:\\Traffic\\";
	m_PathW = m_Root + m_FolderW;
	m_PathQ = m_Root + m_FolderQ;
*/	
	m_Path = g_ConfigData.Gen.TRAFFIC_FOLDER;
	// check if the path ends in a \ and if not add it
	if( *m_Path.rbegin() != '\\')
		m_Path += "\\";

}

CTrafficFiles::~CTrafficFiles()
{

}

void CTrafficFiles::ReadAll(int HWmodel)
{
	ReadPhysical();

	//ReadFile_CP();
	//ReadFile_Q();

	if(HWmodel == 0)
		ReadFile_NHM(); // NHM
}

void CTrafficFiles::ReadPhysical()
{
	ReadFile_AW23();
	ReadFile_AW45();
	ReadFile_AS();
	ReadFile_GVW();
	ReadFile_ATW();
}

//	siteQ=1 !1=Angers=a 2=Auxerre=b 3=A196modified=c
//	siteW=1 !1=Angers=a 2=Auxerre=c 3=A196=b

/*

// Axle Weights for 2/3 axle trucks
// file no. 1
AW[]	-> AW2&3.csv  
	AW[no. of axles, axle no, percentage, Mean, SD]

// Axle Weights for 4/5 axle trucks
// file no. 2
Paw[]	-> AW4&5.csv  
	Paw[no. of axles, interval, Mean W1, Mean W2, Mean WT, SD W1, SD W2, SD WT]

// Axles Spacings 
// file no. 3
AS[]	-> ASall.csv
	AS[no. axles, dist. no., proportion, Mean, SD]

// Truck GVW 
// file no. 4
W[]		-> GVWpdf.csv
	W[direction,	1 is speed,			1 for speed		mean (dm/s),SD (dm/s)]
	W[direction,	2 to 5 no. axles,	proportion,		Mean,		SD]

// % of Trucks per class
// file no. 5
ClassPerc[]	-> Class%.csv
	ClassPerc[direction, truck class]

// Average Hourly Flow Rates (AHFR)
// file no. 6
Q[]	-> FlowR.csv
	Q[direction, hour]

// New Headway Model parameters
// file no. 7
vNHM[]	-> NHM.csv
	vNHM[see paper/manual]

*/

void CTrafficFiles::ReadFile_AW23()
{
	//string file = m_PathW + "AW2&3.csv";
	string file = m_Path + "AW2&3.csv";
	m_CSV.OpenFile(file, ",");
	
	for(int iTruck = 2; iTruck <=3; iTruck++)	// for each truck type
	{
		string line;
		CTriModalNormal temp;
		vector<CTriModalNormal> vAxle(iTruck, temp);

		for(int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for(int j = 0; j < iTruck; j++)	// extract modes of iTruck axles
			{
				double param[3];
				int start = 3*j;
				int end = 3*j+3;
				for(int k = start; k < end; k++)
					param[k-3*j] = m_CSV.stringToDouble( m_CSV.getfield(k) );
				vAxle[j].AddMode(param[0], param[1], param[2]);
			}
		}
		if(iTruck == 2)
		{
			m_TD.Add2AxleWeight(vAxle);	// and add to data store
			m_CSV.getline(line);		// skip blank line
		}
		else
			m_TD.Add3AxleWeight(vAxle);	// add to data store
	}

	m_CSV.CloseFile();			// and close file
}

void CTrafficFiles::ReadFile_AW45()
{
	string file = m_Path + "AW4&5.csv";
	//string file = m_PathW + "AW4&5.csv";
	m_CSV.OpenFile(file, ",");
	
	for(int iTruck = 4; iTruck <=5; iTruck++)	// for each truck type
	{
		string line;
		vector<double> vData(6,0.0);

		for(int iRange = 0; iRange < 12; iRange++)		// for 12 lines of data
		{
			m_CSV.getline(line);
			for(int j = 0; j < 6; j++)			// extract 6 pieces of data
				vData[j] = m_CSV.stringToDouble( m_CSV.getfield(j) );
			m_TD.Add45AxleWeight(vData,iTruck,iRange);
		}
		
		if(iTruck == 4)
			m_CSV.getline(line);		// skip blank line
	}
	
	m_CSV.CloseFile();			// and close file
}


void CTrafficFiles::ReadFile_AS()
{
	string file = m_Path + "ASall.csv";
	//string file = m_PathW + "ASall.csv";
	m_CSV.OpenFile(file, ",");
    
	for(int iTruck = 2; iTruck <=5; iTruck++)	// for each truck type
	{
		string line;
		CTriModalNormal temp;
		vector<CTriModalNormal> vSpace(iTruck, temp);

		for(int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for(int j = 0; j < iTruck-1; j++)	// extract modes of iTruck spacings
			{
				double param[3];
				int start = 3*j;
				int end = 3*j+3;
				for(int k = start; k < end; k++)
					param[k-3*j] = m_CSV.stringToDouble( m_CSV.getfield(k) );
				vSpace[j].AddMode(param[0], param[1], param[2]);
			}
		}

		switch(iTruck)
		{
		case 2:
			m_TD.Add2AxleSpacings(vSpace);	// and add to data store
			break;
		case 3:
			m_TD.Add3AxleSpacings(vSpace);	// and add to data store
			break;
		case 4:
			m_TD.Add4AxleSpacings(vSpace);	// and add to data store
			break;
		case 5:
			m_TD.Add5AxleSpacings(vSpace);	// and add to data store
			break;
		}

		if(iTruck < 5)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}

void CTrafficFiles::ReadFile_ATW()
{
	string file = m_Path + "ATW.csv";
	if(!m_CSV.OpenFile(file, ","))
	{
		std::cout << "*** Warning: Axle track width file not found, using default values" << std::endl;
		CTriModalNormal tmn;
		tmn.AddMode(1,g_ConfigData.Gen.TRUCK_TRACK_WIDTH,0);	// deterministic width
		vector<CTriModalNormal> vTrack(2, tmn); // start off with 2 axles
		m_TD.Add2AxleTrackWidth(vTrack); // add it
		vTrack.push_back(tmn);	m_TD.Add3AxleTrackWidth(vTrack); // add another axle and store it
		vTrack.push_back(tmn);	m_TD.Add4AxleTrackWidth(vTrack); // add another axle and store it
		vTrack.push_back(tmn);	m_TD.Add5AxleTrackWidth(vTrack); // add another axle and store it
		return;	// end the function here
	}

    // only gets here if file exists
	for(int iTruck = 2; iTruck <=5; iTruck++)	// for each truck type
	{
		string line;
		CTriModalNormal temp;
		vector<CTriModalNormal> vTrack(iTruck, temp);

		for(int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for(int j = 0; j < iTruck; j++)	// extract modes of iTruck spacings
			{
				double param[3];
				int start = 3*j;
				int end = 3*j+3;
				for(int k = start; k < end; k++)
					param[k-3*j] = m_CSV.stringToDouble( m_CSV.getfield(k) );
				vTrack[j].AddMode(param[0], param[1], param[2]);
			}
		}

		switch(iTruck)
		{
		case 2:
			m_TD.Add2AxleTrackWidth(vTrack);	// and add to data store
			break;
		case 3:
			m_TD.Add3AxleTrackWidth(vTrack);	// and add to data store
			break;
		case 4:
			m_TD.Add4AxleTrackWidth(vTrack);	// and add to data store
			break;
		case 5:
			m_TD.Add5AxleTrackWidth(vTrack);	// and add to data store
			break;
		}

		if(iTruck < 5)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}

void CTrafficFiles::ReadFile_GVW()
{
	string file = m_Path + "GVWpdf.csv";
	m_CSV.OpenFile(file, ",");
	
	string line;
	CTriModalNormal temp;
	vector<CTriModalNormal> vSpeed(2, temp);
	
	for(int dir = 1; dir <= 2; dir++)
	{
		vector<CTriModalNormal> vGVW(4, temp);

		for(int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for(int j = 0; j < 5; j++)
			{
				double param[3];
				int start = 3*j;
				int end = 3*j+3;
				for(int k = start; k < end; k++)
					param[k-3*j] = m_CSV.stringToDouble( m_CSV.getfield(k) );
				if(j == 0)
					vSpeed[dir-1].AddMode(param[0], param[1], param[2]);	// Speed
				else
					vGVW[j-1].AddMode(param[0], param[1], param[2]);		// GVW
			}
		}
		m_TD.AddGVW(dir, vGVW);
		if(dir == 1)
			m_CSV.getline(line);
	}
	
	m_TD.AddSpeed(vSpeed);
	m_CSV.CloseFile();
}
/*
void CTrafficFiles::ReadFile_CP()
{
	string file = m_PathW + "Class%.csv";	
	m_CSV.OpenFile(file, ",");
    
	for(int i = 0; i < 4; i++)
	{ 
		string line;
		m_CSV.getline(line);
		int nLanes = m_CSV.getnfield()-1;	// minus 1 because the comma at the end is counted
		for(int j = 0; j < nLanes; j++)
			m_TD.AddClassPercent(j, i+2, m_CSV.stringToDouble(m_CSV.getfield(j)) );
	};

	m_CSV.CloseFile();			// and close file
}

void CTrafficFiles::ReadFile_Q()
{
	string file = m_PathQ + "FlowR.csv";
	m_CSV.OpenFile(file, ",");
	
	string line;
	m_CSV.getline(line);
	int nLanes = m_CSV.getnfield() - 1;	// minus 1 because the end comma is counted

	vector<double> temp;
	vector< vector<double> > vFlowRate(nLanes, temp);

	for(int hr = 0; hr < 24; hr++)
	{	
	 	for(int iLane = 0; iLane < nLanes; iLane++)
		{
			string str = m_CSV.getfield(iLane);
			double q = m_CSV.stringToDouble( str );
			vFlowRate[iLane].push_back(q);
		}
		if(hr < 23)
			m_CSV.getline(line);	// at end since line has already been read
	}

	m_TD.SetFlowRate( vFlowRate );
	m_CSV.CloseFile();
}
*/
void CTrafficFiles::ReadFile_NHM()
{
	string file = m_Path + "NHM.csv";
	m_CSV.OpenFile(file, ",");

	std::vector< std::vector<double> > NHM;

	int noRows = 0;
	string line;

	// get no of rows
	m_CSV.getline(line);
	noRows = m_CSV.stringToInt( m_CSV.getfield(0) ) + 3;

	std::vector<double> temp2(noRows,0.0);
	NHM.assign(noRows,temp2);

	NHM[0][0] = noRows - 3; // ie the no of flow intervals
	
	for(int i = 1; i < noRows; i++) // first row already done
	{
		m_CSV.getline(line);
		NHM[i] = m_CSV.GetVectorFromCurrentLine();
	};

	m_TD.SetNHM( NHM );
	m_CSV.CloseFile();
}

CTrafficData CTrafficFiles::GetTrafficData()
{
	return m_TD;
}


std::vector<CLaneFlow> CTrafficFiles::ReadLaneFlow(std::string file)
{
	std::vector<CLaneFlow> vLaneFlow;

	m_CSV.OpenFile(file, ",");
	string line;

	while(m_CSV.getline(line))	// while another lane
	{
		CLaneFlow lane_flow;
		lane_flow.setLaneNo( m_CSV.stringToInt( m_CSV.getfield(0) ) -1 ); // -1: zero-based
		lane_flow.setDirn( m_CSV.stringToInt( m_CSV.getfield(1) ) );
		for (int iHour = 0; iHour < 24; ++iHour)
		{
			if(m_CSV.getline(line))
				lane_flow.setHourData( m_CSV.GetVectorFromCurrentLine() );
			else
				cout << "ERROR: not enough hours in lane " << lane_flow.getLaneNo() 
					 << "  - no flow data beyond hour " << iHour << endl;
		}
		vLaneFlow.push_back(lane_flow);
	}
	m_CSV.CloseFile();
	return vLaneFlow;
}