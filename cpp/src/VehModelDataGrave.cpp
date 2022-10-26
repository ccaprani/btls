#include "VehModelDataGrave.h"
#include "ConfigData.h"


CVehModelDataGrave::CVehModelDataGrave(CVehicleClassification_sp pVC, CLaneFlowComposition lfc)
	: CVehicleModelData(eVM_Grave, pVC, lfc, 5) // MAGIC NUMBER - truck class count of the Grave Model
{
	GVWRange range;
	m_v4AxleWeight.assign(12, range);
	m_v5AxleWeight.assign(12, range);

	ReadDataIn();
}

CVehModelDataGrave::CVehModelDataGrave(CVehicleClassification_sp pVC, CLaneFlowComposition lfc, CPyConfigData& pyConfig)
	: CVehicleModelData(eVM_Grave, pVC, lfc, 5, pyConfig) // MAGIC NUMBER - truck class count of the Grave Model
{
	GVWRange range;
	m_v4AxleWeight.assign(12, range);
	m_v5AxleWeight.assign(12, range);

	CConfigData::get().Gen.TRUCK_TRACK_WIDTH = pyConfig.Gen_TRUCK_TRACK_WIDTH;

	ReadDataIn();
}

CVehModelDataGrave::~CVehModelDataGrave()
{

}

////////// THE GETS ////////////////

std::vector<double>	CVehModelDataGrave::GetGVWRange(size_t iTruck, size_t iRange)
{
	std::vector<double> data;
	GVWRange range;
	if(iRange > 11)
		iRange = 11;
	if( iTruck == 4)
		range = m_v4AxleWeight[iRange];
	else
		range = m_v5AxleWeight[iRange];

	data.push_back(	range.W1.Mean );
	data.push_back(	range.W2.Mean );
	data.push_back(	range.WT.Mean );
	data.push_back(	range.W1.StdDev );
	data.push_back(	range.W2.StdDev );
	data.push_back(	range.WT.StdDev );

	return data;
}

CTriModalNormal	CVehModelDataGrave::GetSpacingDist(size_t iTruck, size_t iSpace)
{
	switch( iTruck )
	{
		case 2:
			return m_v2AxleSpacings[iSpace];
		case 3:
			return m_v3AxleSpacings[iSpace];
		case 4:
			return m_v4AxleSpacings[iSpace];
		default:
			return m_v5AxleSpacings[iSpace];
	}
}

CTriModalNormal	CVehModelDataGrave::GetAxleWeightDist(size_t iTruck, size_t iAxle)
{
	if( iTruck == 2 )
		return m_v2AxleWeight[iAxle];
	else
		return m_v3AxleWeight[iAxle];
}

CTriModalNormal	CVehModelDataGrave::GetTrackWidthDist(size_t iTruck, size_t iAxle)
{
	switch( iTruck )
	{
		case 2:
			return m_v2AxleTrackWidth[iAxle];
		case 3:
			return m_v3AxleTrackWidth[iAxle];
		case 4:
			return m_v4AxleTrackWidth[iAxle];
		default:
			return m_v5AxleTrackWidth[iAxle];
	}
}

CTriModalNormal	CVehModelDataGrave::GetGVW(size_t dir, size_t iTruck)
{
	if(dir == 1)
		return m_vDir1GVW[iTruck - 2];
	else
		return m_vDir2GVW[iTruck - 2];
}

CTriModalNormal CVehModelDataGrave::GetSpeed(std::size_t dir)
{
	if (dir == 1)
		return m_vSpeed[0];
	else
		return m_vSpeed[1];
}

/////////////// THE SETS //////////////////

void CVehModelDataGrave::Add2AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v2AxleSpacings = vSpace;
}

void CVehModelDataGrave::Add3AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v3AxleSpacings = vSpace;
}

void CVehModelDataGrave::Add4AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v4AxleSpacings = vSpace;
}

void CVehModelDataGrave::Add5AxleSpacings(std::vector<CTriModalNormal> vSpace)
{
	m_v5AxleSpacings = vSpace;
}

void CVehModelDataGrave::Add2AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v2AxleTrackWidth = vTrack;
}

void CVehModelDataGrave::Add3AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v3AxleTrackWidth = vTrack;
}

void CVehModelDataGrave::Add4AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v4AxleTrackWidth = vTrack;
}

void CVehModelDataGrave::Add5AxleTrackWidth(std::vector<CTriModalNormal> vTrack)
{
	m_v5AxleTrackWidth = vTrack;
}

void CVehModelDataGrave::Add2AxleWeight(std::vector<CTriModalNormal> vAxle)
{
	m_v2AxleWeight = vAxle;
}

void CVehModelDataGrave::Add3AxleWeight(std::vector<CTriModalNormal> vAxle)
{
	m_v3AxleWeight = vAxle;
}

void CVehModelDataGrave::Add45AxleWeight(std::vector<double> data, std::size_t iTruck, std::size_t iRange)
{
	GVWRange range;
	range.W1.Mean = data[0];
	range.W2.Mean = data[1];
	range.WT.Mean = data[2];
	range.W1.StdDev = data[3];
	range.W2.StdDev = data[4];
	range.WT.StdDev = data[5];

	if( iTruck == 4)
		m_v4AxleWeight[iRange] = range;
	else
		m_v5AxleWeight[iRange] = range;
}

void CVehModelDataGrave::AddGVW(int dir, std::vector<CTriModalNormal> vGVW)
{
	if(dir == 1)
		m_vDir1GVW = vGVW;
	else
		m_vDir2GVW = vGVW;
}

void CVehModelDataGrave::AddSpeed(std::vector<CTriModalNormal> vSpeed)
{
	m_vSpeed = vSpeed;
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
	filesystem::path file = m_Path / "AW2&3.csv";
	if( !m_CSV.OpenFile(file.string(), ",") )
	{
		std::cerr << "***ERROR: " << std::filesystem::weakly_canonical(file) << " file could not be opened" << endl;
		system("PAUSE");
		exit( 1 );
	}

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
			Add2AxleWeight(vAxle);	// and add to data store
			m_CSV.getline(line);		// skip blank line
		}
		else
			Add3AxleWeight(vAxle);	// add to data store
	}

	m_CSV.CloseFile();			// and close file
}

void CVehModelDataGrave::ReadFile_AW45()
{
	filesystem::path file = m_Path / "AW4&5.csv";
	if( !m_CSV.OpenFile(file.string(), ",") )
	{
		std::cerr << "***ERROR: " << std::filesystem::weakly_canonical(file) << " file could not be opened" << endl;
		system("PAUSE");
		exit( 1 );
	}

	for (int iTruck = 4; iTruck <= 5; iTruck++)	// for each truck type
	{
		string line;
		vector<double> vData(6, 0.0);

		for (int iRange = 0; iRange < 12; iRange++)		// for 12 lines of data
		{
			m_CSV.getline(line);
			for (int j = 0; j < 6; j++)			// extract 6 pieces of data
				vData[j] = m_CSV.stringToDouble(m_CSV.getfield(j));
			Add45AxleWeight(vData, iTruck, iRange);
		}

		if (iTruck == 4)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}


void CVehModelDataGrave::ReadFile_AS()
{
	filesystem::path  file = m_Path / "ASall.csv";
	if( !m_CSV.OpenFile(file.string(), ",") )
	{
		std::cerr << "***ERROR: " << std::filesystem::weakly_canonical(file) << " file could not be opened" << endl;
		system("PAUSE");
		exit( 1 );
	}

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
			Add2AxleSpacings(vSpace);	// and add to data store
			break;
		case 3:
			Add3AxleSpacings(vSpace);	// and add to data store
			break;
		case 4:
			Add4AxleSpacings(vSpace);	// and add to data store
			break;
		case 5:
			Add5AxleSpacings(vSpace);	// and add to data store
			break;
		}

		if (iTruck < 5)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}

void CVehModelDataGrave::ReadFile_ATW()
{
	filesystem::path  file = m_Path / "ATW.csv";
	if (!m_CSV.OpenFile(file.string(), ","))
	{
		std::cout << "*** Warning: Axle track width file not found: " 
				  << std::filesystem::weakly_canonical(file) 
				  << " using default values" << std::endl;
		CTriModalNormal tmn;
		tmn.AddMode(1, CConfigData::get().Gen.TRUCK_TRACK_WIDTH, 0);	// deterministic width
		vector<CTriModalNormal> vTrack(2, tmn); // start off with 2 axles
		Add2AxleTrackWidth(vTrack); // add it
		vTrack.push_back(tmn);	Add3AxleTrackWidth(vTrack); // add another axle and store it
		vTrack.push_back(tmn);	Add4AxleTrackWidth(vTrack); // add another axle and store it
		vTrack.push_back(tmn);	Add5AxleTrackWidth(vTrack); // add another axle and store it
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
			Add2AxleTrackWidth(vTrack);	// and add to data store
			break;
		case 3:
			Add3AxleTrackWidth(vTrack);	// and add to data store
			break;
		case 4:
			Add4AxleTrackWidth(vTrack);	// and add to data store
			break;
		case 5:
			Add5AxleTrackWidth(vTrack);	// and add to data store
			break;
		}

		if (iTruck < 5)
			m_CSV.getline(line);		// skip blank line
	}

	m_CSV.CloseFile();			// and close file
}

void CVehModelDataGrave::ReadFile_GVW()
{
	filesystem::path  file = m_Path / "GVWpdf.csv";
	if( !m_CSV.OpenFile(file.string(), ",") )
	{
		std::cerr << "***ERROR: " << std::filesystem::weakly_canonical(file) << " file could not be opened" << endl;
		system("PAUSE");
		exit( 1 );
	}

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
		AddGVW(dir, vGVW);
		if (dir == 1)
			m_CSV.getline(line);
	}

	AddSpeed(vSpeed);
	m_CSV.CloseFile();
}
