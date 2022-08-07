#include "ReadILFile.h"

using namespace std;

CReadILFile::CReadILFile(void)
	: m_NoInfLines(0)
{
}

CReadILFile::CReadILFile(string file)
	: m_NoInfLines(0)
{
	ReadILFile(file);
}

CReadILFile::CReadILFile(std::string file, unsigned int mode)
	: m_NoInfLines(0)
{
	if(mode == 1)	// read an influence surface
		ReadInfSurfFile(file);
	else
		ReadILFile(file);
}

CReadILFile::~CReadILFile(void)
{
}


// Reads the IL file in and sets the distance and ordinate vectors
void CReadILFile::ReadILFile(string file)
{
	m_File = file;
	m_InFileStream.open(m_File, ios::in );
	if( !m_InFileStream )
	{
		std::cerr << "*** Warning: Influence Line file could not be opened" << endl;
		return;
	}	
	
	string temp;
	getline(m_InFileStream, temp);
	m_NoInfLines = stringToInt(temp);

	for(int i = 0; i < m_NoInfLines; i++)
	{
		CInfluenceLine infline;
		getline(m_InFileStream, temp,',');
		infline.setIndex( stringToInt(temp) );
		getline(m_InFileStream, temp,'\n');
		int nPoints = stringToInt(temp);
		std::vector<double> vDis(nPoints,0.0);
		std::vector<double> vOrd(nPoints,0.0);
		for(int j = 0; j < nPoints; j++)
		{
			getline(m_InFileStream, temp, ',');		
			vDis.at(j) = stringToDouble(temp);
			getline(m_InFileStream, temp, '\n');	
			vOrd.at(j) = stringToDouble(temp);
		}
		infline.setIL(vDis,vOrd);
		m_vInfLine.push_back(infline);
	}
	m_InFileStream.close();
}


// Reads an Influence Surface file in
void CReadILFile::ReadInfSurfFile(string file)
{
	if( !m_CSV.OpenFile(file, ",") )
	{
		std::cerr << "*** Warning: Influence Surface file could not be opened" << endl;
		return;
	}
	string line;

	while(m_CSV.getline(line))	// while another IS
	{
		CInfluenceLine infline;
		CInfluenceSurface IS;
		infline.setIndex( m_CSV.stringToInt( m_CSV.getfield(0) ) );
		size_t nrows = m_CSV.stringToInt(m_CSV.getfield(1));
		size_t nLanes = m_CSV.stringToInt(m_CSV.getfield(2));
		// set the lanes
		std::vector<double> ylanes;
		for (size_t i = 0; i <= nLanes; i++)
			ylanes.push_back( m_CSV.stringToDouble( m_CSV.getfield(i+3) ) );
		IS.setLanes(ylanes);
		// read in the ISmatrix
		std::vector<std::vector<double>> ISmat;
		for (size_t i = 0; i < nrows + 1; i++)
			ISmat.push_back( m_CSV.GetVectorFromNextLine() );
		// set it and assign IS to IL
		IS.setIS(ISmat);
		infline.setIL(IS);
		m_vInfLine.push_back(infline);
	}
}

vector<CInfluenceLine> CReadILFile::getInfLines()
{
	return m_vInfLine;
}

std::vector<CInfluenceLine> CReadILFile::getInfLines(std::string file,unsigned int mode)
{
	m_vInfLine.clear();	// dump whatever we have
	m_NoInfLines = 0;

	if (mode == 1)	// read an influence surface
	{
		cout << "Reading influence surface file: " << file.c_str() << endl;
		ReadInfSurfFile(file);
	}

	else
	{
		cout << "Reading influence line file: " << file.c_str() << endl;
		ReadILFile(file);
	}
	
	return m_vInfLine;
}

double CReadILFile::stringToDouble(string line)
{
	stringstream ss(line);
	double n;
	ss >> n;
	return n;
}

int CReadILFile::stringToInt(string line)
{
	stringstream ss(line);
	int n;
	ss >> n;
	return n;
}

int CReadILFile::getNoInfLines(void)
{
	return m_NoInfLines;
}
