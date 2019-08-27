#include "VehModelDataGrave.h"
#include "ConfigData.h"

extern CConfigData g_ConfigData;

/////////////// CVehicleModelGrave /////////////////

CVehModelDataGrave::CVehModelDataGrave(CVehicleClassification* pVC, CLaneFlowComposition lfc)
	: CVehicleModelData(eGrave, pVC, lfc, 5) // MAGIC NUMBER - truck class count of the Grave Model
{
	ReadDataIn();
}

CVehModelDataGrave::~CVehModelDataGrave()
{

}

std::vector<double>	CVehModelDataGrave::GetGVWRange(int iTruck, int iRange)
{
	return m_TD.GetGVWRange(iTruck, iRange);
}


CTriModalNormal	CVehModelDataGrave::GetSpacingDist(int iTruck, int iSpace)
{
	return m_TD.GetSpacingDist(iTruck, iSpace);
}


CTriModalNormal	CVehModelDataGrave::GetAxleWeightDist(int iTruck, int iAxle)
{
	return m_TD.GetAxleWeightDist(iTruck, iAxle);
}


CTriModalNormal	CVehModelDataGrave::GetTrackWidthDist(int iTruck, int iAxle)
{
	return m_TD.GetTrackWidthDist(iTruck, iAxle);
}


CTriModalNormal	CVehModelDataGrave::GetGVW(int dir, int iTruck)
{
	return m_TD.GetGVW(dir, iTruck);
}

void CVehModelDataGrave::ReadDataIn()
{
	ReadFile_AW23();
	ReadFile_AW45();
	ReadFile_AS();
	ReadFile_GVW();
	ReadFile_ATW();
}


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

void CVehModelDataGrave::ReadFile_AW23()
{
	//string file = m_PathW + "AW2&3.csv";
	string file = m_Path + "AW2&3.csv";
	m_CSV.OpenFile(file, ",");

	for (int iTruck = 2; iTruck <= 3; iTruck++)	// for each truck type
	{
		string line;
		CTriModalNormal temp;
		vector<CTriModalNormal> vAxle(iTruck, temp);

		for (int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for (int j = 0; j < iTruck; j++)	// extract modes of iTruck axles
			{
				double param[3];
				int start = 3 * j;
				int end = 3 * j + 3;
				for (int k = start; k < end; k++)
					param[k - 3 * j] = m_CSV.stringToDouble(m_CSV.getfield(k));
				vAxle[j].AddMode(param[0], param[1], param[2]);
			}
		}
		if (iTruck == 2)
		{
			m_TD.Add2AxleWeight(vAxle);	// and add to data store
			m_CSV.getline(line);		// skip blank line
		}
		else
			m_TD.Add3AxleWeight(vAxle);	// add to data store
	}

	m_CSV.CloseFile();			// and close file
}

void CVehModelDataGrave::ReadFile_AW45()
{
	string file = m_Path + "AW4&5.csv";
	//string file = m_PathW + "AW4&5.csv";
	m_CSV.OpenFile(file, ",");

	for (int iTruck = 4; iTruck <= 5; iTruck++)	// for each truck type
	{
		string line;
		vector<double> vData(6, 0.0);

		for (int iRange = 0; iRange < 12; iRange++)		// for 12 lines of data
		{
			m_CSV.getline(line);
			for (int j = 0; j < 6; j++)			// extract 6 pieces of data
				vData[j] = m_CSV.stringToDouble(m_CSV.getfield(j));
			m_TD.Add45AxleWeight(vData, iTruck, iRange);
		}

		if (iTruck == 4)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}


void CVehModelDataGrave::ReadFile_AS()
{
	string file = m_Path + "ASall.csv";
	//string file = m_PathW + "ASall.csv";
	m_CSV.OpenFile(file, ",");

	for (int iTruck = 2; iTruck <= 5; iTruck++)	// for each truck type
	{
		string line;
		CTriModalNormal temp;
		vector<CTriModalNormal> vSpace(iTruck, temp);

		for (int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for (int j = 0; j < iTruck - 1; j++)	// extract modes of iTruck spacings
			{
				double param[3];
				int start = 3 * j;
				int end = 3 * j + 3;
				for (int k = start; k < end; k++)
					param[k - 3 * j] = m_CSV.stringToDouble(m_CSV.getfield(k));
				vSpace[j].AddMode(param[0], param[1], param[2]);
			}
		}

		switch (iTruck)
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

		if (iTruck < 5)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}

void CVehModelDataGrave::ReadFile_ATW()
{
	string file = m_Path + "ATW.csv";
	if (!m_CSV.OpenFile(file, ","))
	{
		std::cout << "*** Warning: Axle track width file not found, using default values" << std::endl;
		CTriModalNormal tmn;
		tmn.AddMode(1, g_ConfigData.Gen.TRUCK_TRACK_WIDTH, 0);	// deterministic width
		vector<CTriModalNormal> vTrack(2, tmn); // start off with 2 axles
		m_TD.Add2AxleTrackWidth(vTrack); // add it
		vTrack.push_back(tmn);	m_TD.Add3AxleTrackWidth(vTrack); // add another axle and store it
		vTrack.push_back(tmn);	m_TD.Add4AxleTrackWidth(vTrack); // add another axle and store it
		vTrack.push_back(tmn);	m_TD.Add5AxleTrackWidth(vTrack); // add another axle and store it
		return;	// end the function here
	}

	// only gets here if file exists
	for (int iTruck = 2; iTruck <= 5; iTruck++)	// for each truck type
	{
		string line;
		CTriModalNormal temp;
		vector<CTriModalNormal> vTrack(iTruck, temp);

		for (int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for (int j = 0; j < iTruck; j++)	// extract modes of iTruck spacings
			{
				double param[3];
				int start = 3 * j;
				int end = 3 * j + 3;
				for (int k = start; k < end; k++)
					param[k - 3 * j] = m_CSV.stringToDouble(m_CSV.getfield(k));
				vTrack[j].AddMode(param[0], param[1], param[2]);
			}
		}

		switch (iTruck)
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

		if (iTruck < 5)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}

void CVehModelDataGrave::ReadFile_GVW()
{
	string file = m_Path + "GVWpdf.csv";
	m_CSV.OpenFile(file, ",");

	string line;
	CTriModalNormal temp;
	vector<CTriModalNormal> vSpeed(2, temp);

	for (int dir = 1; dir <= 2; dir++)
	{
		vector<CTriModalNormal> vGVW(4, temp);

		for (int i = 0; i < 3; i++)		// for 3 lines of data
		{
			m_CSV.getline(line);
			for (int j = 0; j < 5; j++)
			{
				double param[3];
				int start = 3 * j;
				int end = 3 * j + 3;
				for (int k = start; k < end; k++)
					param[k - 3 * j] = m_CSV.stringToDouble(m_CSV.getfield(k));
				if (j == 0)
					vSpeed[dir - 1].AddMode(param[0], param[1], param[2]);	// Speed
				else
					vGVW[j - 1].AddMode(param[0], param[1], param[2]);		// GVW
			}
		}
		m_TD.AddGVW(dir, vGVW);
		if (dir == 1)
			m_CSV.getline(line);
	}

	m_TD.AddSpeed(vSpeed);
	m_CSV.CloseFile();
}
