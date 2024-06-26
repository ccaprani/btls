// CSVParse.cpp: implementation of the CCSVParse class.
//
//////////////////////////////////////////////////////////////////////

#include "CSVParse.h"

#ifdef WIN_DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCSVParse::CCSVParse()
{
		
}

CCSVParse::~CCSVParse()
{

}

/*	EXAMPLE USE
// last line of file must be blank for this class to open a different file
// Csvtest main: test Csv class

int main(void)
{
	string line;
	Csv csv;

	while (csv.getline(line) != 0) {
		cout << "line = `" << line <<"'\n";
		for (int i = 0; i < csv.getnfield(); i++)
			cout << "field[" << i << "] = `"
				 << csv.getfield(i) << "'\n";
	}
	return 0;
}
*/

bool CCSVParse::OpenFile(std::string inFile, std::string sep)
{
	fin.open( inFile.c_str(), std::ios::in );

	if(!fin)
		return false;
	
	fieldsep = sep;
	return true;
}

void CCSVParse::CloseFile()
{
	fin.close();
}

// endofline: check for and consume \r, \n, \r\n, or EOF
int CCSVParse::endofline(char c)
{
	int eol;

	eol = (c=='\r' || c=='\n');
	if (c == '\r') {
		fin.get(c);
		if (!fin.eof() && c != '\n')
			fin.putback(c);	// read too far
	}
	return eol;
}

// getline: get one line, grow as needed
int CCSVParse::getline(std::string& str)
{	
	char c;

	for (line = ""; fin.get(c) && !endofline(c); )
		line += c;
	split();
	str = line;
	return !fin.eof();
}

// split: split line into fields
size_t CCSVParse::split()
{
	std::string fld;
	size_t i, j;

	nfield = 0;
	if (line.length() == 0)
		return 0;
	i = 0;

	do {
		if (i < line.length() && line[i] == '"')
			j = advquoted(line, fld, ++i);	// skip quote
		else
			j = advplain(line, fld, i);
		if (nfield >= field.size())
			field.push_back(fld);
		else
			field[nfield] = fld;
		nfield++;
		i = j + 1;
	} while (j < line.length());

	return nfield;
}

// advquoted: quoted field; return index of next separator
size_t CCSVParse::advquoted(const std::string& s, std::string& fld, size_t i)
{
	size_t j;

	fld = "";
	for (j = i; j < s.length(); j++) {
		if (s[j] == '"' && s[++j] != '"') {
			size_t k = s.find_first_of(fieldsep, j);
			if (k > s.length())	// no separator found
				k = s.length();
			for (k -= j; k-- > 0; )
				fld += s[j++];
			break;
		}
		fld += s[j];
	}
	return j;
}

// advplain: unquoted field; return index of next separator
size_t CCSVParse::advplain(const std::string& s, std::string& fld, size_t i)
{
	size_t j;

	j = s.find_first_of(fieldsep, i); // look for separator
	if (j > s.length())               // none found
		j = s.length();
	fld = std::string(s, i, j-i);
	return j;
}


// getfield: return n-th field
std::string CCSVParse::getfield(size_t n)
{
	//if (n < 0 || n >= nfield)
	if (n >= nfield)
		return "";
	else
		return field[n];
}

std::vector<double> CCSVParse::GetVectorFromNextLine()
{
	std::vector<double> vData;
	std::string line;
	getline(line);

	size_t nData = getnfield();
	for(size_t j = 0; j < nData; j++)
	{
		std::string str = getfield(j);
		double val = stringToDouble(str);
		vData.push_back(val);
	}

	return vData;
}

std::vector<double> CCSVParse::GetVectorFromCurrentLine()
{
	std::vector<double> vData;

	for (size_t j = 0; j < nfield; j++)
	{
		std::string str = getfield(j);
		double val = stringToDouble(str);
		vData.push_back(val);
	}

	return vData;
}

double CCSVParse::stringToDouble(std::string line)
{
	double val = 0.0;
	std::istringstream stream (line);
	if (stream >> val) 
		return val;
	else
		std::cout << "Conversion Error";
	return val;
}

int CCSVParse::stringToInt(std::string line)
{
/*	
	int val = 0;	char data[20]; 
	line.copy(data, 19, 0);  
	val = atoi(data);
	return val;
*/
	int val = 0;
	std::istringstream stream (line);
	if (stream >> val) 
		return val;
	else
		std::cout << "Conversion Error";
	return val;
}

bool CCSVParse::stringToBool(std::string line)
{
	int val = stringToInt(line);
	if(val == 1)
		return true;
	else
		return false;
}

/*
template<class T>
std::string ToString(const T& val)
{
    std::stringstream<char> strm;
    strm << val;
    return strm.str();
}

template <typename T>
T CCSVParse::from_string (const std::string & s)
{
	T result;
	std::istringstream stream (s);
	if (stream >> result) return result;
	throw conversion_failure ();
}
*/
