#include "VehicleTrafficFile.h"
#include "CSVParse.h"
#include "TrafficFileFormat.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include <unordered_map>

namespace
{

	bool isMissing(const std::string& value)
	{
		std::string s = value;
		s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
		return s.empty() || s == "-" || s == "nan" || s == "NaN";
	}

	double parseDouble(const std::string& value, double defaultValue = 0.0)
	{
		if (isMissing(value)) return defaultValue;
		std::string s = value;
		s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
		std::replace(s.begin(), s.end(), ',', '.');
		return std::stod(s);
	}

	size_t parseSizeT(const std::string& value, size_t defaultValue = 0)
	{
		return static_cast<size_t>(std::llround(parseDouble(value, static_cast<double>(defaultValue))));
	}

	std::string getField(
		const std::vector<std::string>& fields,
		const std::unordered_map<std::string, size_t>& header,
		const std::string& name,
		const std::string& defaultValue = "")
	{
		auto it = header.find(name);
		if (it == header.end() || it->second >= fields.size()) return defaultValue;
		return fields[it->second];
	}

	double requireDouble(
		const std::vector<std::string>& fields,
		const std::unordered_map<std::string, size_t>& header,
		const std::string& name,
		size_t rowNo)
	{
		std::string value = getField(fields, header, name);
		if (isMissing(value))
			throw std::invalid_argument("SiWIM row " + std::to_string(rowNo) + " has no value in mandatory column " + name);
		return parseDouble(value);
	}

	void parseSiwimTimestamp(const std::string& timestamp, size_t& year, size_t& month, size_t& day, size_t& hour, size_t& min, double& sec)
	{
		if (timestamp.size() < 23)
			throw std::invalid_argument("Bad SiWIM timestamp: " + timestamp);
		year = parseSizeT(timestamp.substr(0, 4));
		month = parseSizeT(timestamp.substr(5, 2));
		day = parseSizeT(timestamp.substr(8, 2));
		hour = parseSizeT(timestamp.substr(11, 2));
		min = parseSizeT(timestamp.substr(14, 2));
		double seconds = parseDouble(timestamp.substr(17, 2));
		double millis = parseDouble(timestamp.substr(20, 3));
		sec = seconds + millis / 1000.0;
	}

	size_t siwimAxleGroupCount(const std::vector<std::string>& fields, const std::unordered_map<std::string, size_t>& header, size_t noAxles)
	{
		std::string raw = getField(fields, header, "wim.axgrps");
		if (!raw.empty())
		{
			try
			{
				size_t axgrps = parseSizeT(raw);
				if (axgrps > 0) return std::to_string(axgrps).size();
			}
			catch (...) {}
		}
		if (noAxles == 0) return 0;
		size_t groups = 1;
		for (size_t i = 0; i + 1 < noAxles; ++i)
		{
			if (parseDouble(getField(fields, header, "wim.ads.d." + std::to_string(i))) > 2.0)
				groups++;
		}
		return groups;
	}

	class CFixedWidthVehicleFileParser : public CVehicleFileParser
	{
	public:
		CFixedWidthVehicleFileParser(std::filesystem::path file, int filetype)
			: m_File(file, std::ios::in), m_Filetype(filetype)
		{
			if (!m_File)
				throw std::runtime_error("Input traffic file could not be opened: " + file.string());
		}

		CVehicle_sp nextVehicle() override
		{
			std::string str;
			while (std::getline(m_File, str))
			{
				if (!str.empty())
				{
					CVehicle_sp pVeh = std::make_shared<CVehicle>();
					pVeh->create(str, m_Filetype);
					return pVeh;
				}
			}
			return nullptr;
		}

	private:
		std::ifstream m_File;
		int m_Filetype;
	};

	class CSiwimVehicleFileParser : public CVehicleFileParser
	{
	public:
		CSiwimVehicleFileParser(std::filesystem::path file, const std::string& delimiter)
		{
			if (!m_CSV.OpenFile(file.string(), delimiter))
				throw std::runtime_error("Input SiWIM file could not be opened: " + file.string());

			std::string line;
			m_Eof = (m_CSV.getline(line) == 0);
			if (line.empty())
				throw std::runtime_error("Input SiWIM file has no header: " + file.string());

			for (size_t i = 0; i < m_CSV.getnfield(); ++i)
				m_Header[m_CSV.getfield(i)] = i;
		}

		CVehicle_sp nextVehicle() override
		{
			while (!m_Eof)
			{
				std::string line;
				m_Eof = (m_CSV.getline(line) == 0);
				if (line.empty()) continue;
				std::vector<std::string> fields;
				fields.reserve(m_CSV.getnfield());
				for (size_t i = 0; i < m_CSV.getnfield(); ++i)
					fields.push_back(m_CSV.getfield(i));
				m_RowNo++;
				return createVehicle(fields);
			}
			return nullptr;
		}

	private:
		CVehicle_sp createVehicle(const std::vector<std::string>& fields)
		{
			std::string timestamp = getField(fields, m_Header, "wim.ts");
			size_t year, month, day, hour, min;
			double sec;
			parseSiwimTimestamp(timestamp, year, month, day, hour, min, sec);

			size_t noAxles = parseSizeT(getField(fields, m_Header, "wim.naxles"));
			if (noAxles < 1)
				throw std::invalid_argument("SiWIM row has invalid axle count");

			double velocity = requireDouble(fields, m_Header, "wim.v", m_RowNo);
			if (velocity <= 0.0)
				throw std::invalid_argument("SiWIM row " + std::to_string(m_RowNo) + " has a non-positive velocity in column wim.v");

			size_t lane = parseSizeT(getField(fields, m_Header, "wim.lane"), 1);
			if (lane > 1 && !m_MultiLaneWarned)
			{
				std::cout << "*** WARNING: SiWIM file uses more than one lane; all vehicles are assigned direction 1" << std::endl;
				m_MultiLaneWarned = true;
			}

			CVehicle_sp pVeh = std::make_shared<CVehicle>();
			pVeh->setNoAxles(noAxles);
			pVeh->setHead(static_cast<int>(m_RowNo));
			pVeh->setDateTime(year, month, day, hour, min, sec);
			pVeh->setLocalLane(lane);
			pVeh->setDirection(1);
			pVeh->setVelocity(velocity);
			pVeh->setGVW(requireDouble(fields, m_Header, "wim.gvw", m_RowNo));
			pVeh->setLength(requireDouble(fields, m_Header, "wim.whlbse", m_RowNo));
			pVeh->setNoAxleGroups(siwimAxleGroupCount(fields, m_Header, noAxles));
			pVeh->setTrans(0.0);

			for (size_t i = 0; i < noAxles; ++i)
			{
				pVeh->setAW(i, requireDouble(fields, m_Header, "wim.acws.w." + std::to_string(i), m_RowNo));
				double spacing = (i + 1 < noAxles) ? parseDouble(getField(fields, m_Header, "wim.ads.d." + std::to_string(i))) : 0.0;
				pVeh->setAS(i, spacing);
			}

			return pVeh;
		}

		CCSVParse m_CSV;
		std::unordered_map<std::string, size_t> m_Header;
		size_t m_RowNo = 0;
		bool m_Eof = false;
		bool m_MultiLaneWarned = false;
	};

	std::unique_ptr<CVehicleFileParser> createVehicleFileParser(std::filesystem::path file, int filetype)
	{
		TrafficFileFormatSpec spec = requireTrafficFileReadFormat(filetype);
		switch (spec.Kind)
		{
		case ETrafficFileKind::FixedWidth:
			return std::make_unique<CFixedWidthVehicleFileParser>(file, filetype);
		case ETrafficFileKind::HeaderCsv:
			if (spec.Format == ETrafficFileFormat::Siwim)
				return std::make_unique<CSiwimVehicleFileParser>(file, spec.Delimiter);
		}
		throw std::invalid_argument(std::string("Traffic file format ") + spec.Name + " does not have a registered reader");
	}
}

CVehicleTrafficFile::CVehicleTrafficFile(CVehicleClassification_sp pVC, 
	bool UseConstSpeed, bool UseAveSpeed, double ConstSpeed)
{
	m_NoVehs = 0;
	m_NoDays = 0;
	m_NoLanes = 0;
	m_NoDirn = 0;
	m_NoLanesDir1 = 0;
	m_NoLanesDir2 = 0;
	
	m_iCurVehicle = 0;

	m_pVehClassification = pVC;
	m_UseConstSpeed = UseConstSpeed;
	m_UseAveSpeed = UseAveSpeed;
	m_ConstSpeed = ConstSpeed;	
}


CVehicleTrafficFile::~CVehicleTrafficFile(void)
{
}

void CVehicleTrafficFile::Read(std::filesystem::path file, int filetype)
{
	m_vVehicles.clear();
	m_iCurVehicle = 0;

	std::unique_ptr<CVehicleFileParser> parser = createVehicleFileParser(file, filetype);

	while (CVehicle_sp pVeh = parser->nextVehicle())
	{
		m_pVehClassification->setClassification(pVeh);
		m_vVehicles.push_back(pVeh);
	}

	AnalyseTraffic();
}

void CVehicleTrafficFile::AssignTraffic(std::vector<CVehicle_sp> vVehicles)
{
	m_vVehicles = vVehicles;
	for (CVehicle_sp pVeh : m_vVehicles)
	{
		m_pVehClassification->setClassification(pVeh);
	}
	AnalyseTraffic();
}

void CVehicleTrafficFile::AnalyseTraffic()
{
	UpdateProperties();
	if(m_UseConstSpeed)
		SetSpeed();
}

void CVehicleTrafficFile::SetSpeed()
{
	
	{
		double speed = 0.0;
		if(m_UseAveSpeed)
		{
			for(unsigned int i = 0; i < m_NoVehs; ++i)
				speed += m_vVehicles.at(i)->getVelocity();
			speed /= m_NoVehs;
		}
		else
			speed = m_ConstSpeed/3.6; // km/h to m/s
		
		for(unsigned int i = 0; i < m_NoVehs; ++i)
			m_vVehicles.at(i)->setVelocity(speed);
	}
}

void CVehicleTrafficFile::UpdateProperties()
{
	m_NoVehs = m_vVehicles.size();

	size_t maxLaneNoDir1 = 0;
	size_t maxLaneNoDir2 = 0;

	for(unsigned int i = 0; i < m_NoVehs; ++i)
	{
		CVehicle_sp pVeh = m_vVehicles.at(i);
		size_t dirn = pVeh->getDirection();
		size_t lane = pVeh->getLocalLane();
		
		if(dirn == 1 && lane > maxLaneNoDir1) maxLaneNoDir1 = lane;
		if(dirn == 2 && lane > maxLaneNoDir2) maxLaneNoDir2 = lane;
	}
	
	// If both directions have lanes
	if(maxLaneNoDir1 > 0 && maxLaneNoDir2 > 0)
	{ 
		m_NoLanesDir1 = maxLaneNoDir1;
		// If vehicle lanes are cumulative then subtract dir 1 lanes from dir 2
		//m_NoLanesDir2 = maxLaneNoDir2 -maxLaneNoDir1;
		// Lnes are no longer cumulative
		m_NoLanesDir2 = maxLaneNoDir2;
		m_NoLanes = m_NoLanesDir1 + m_NoLanesDir2;
		m_NoDirn = 2;
	}
	else	// only one direction has lanes
	{
		if(maxLaneNoDir1 > 0)	// it's direction 1
		{
			m_NoLanesDir1 = maxLaneNoDir1;
			m_NoLanes = m_NoLanesDir1;
		}
		else // it's direction 2
		{
			m_NoLanesDir2 = maxLaneNoDir2;
			m_NoLanes = m_NoLanesDir2;
		}
		m_NoDirn = 1;
	}

	if(m_NoVehs > 0)
	{
		m_Starttime = m_vVehicles.front()->getTime();
		m_Endtime = m_vVehicles.back()->getTime();
		m_NoDays = (int)((m_Endtime - m_Starttime)/(3600.0*24)) + 1;
	}
	else
		std::cout << "*** ERROR: No vehicles in traffic file" << std::endl;
}

CVehicle_sp CVehicleTrafficFile::getNextVehicle()
{
	if(m_iCurVehicle > m_NoVehs-1) // m_iCurVehicle is zero-based
		return nullptr;

	CVehicle_sp pVeh = std::make_shared<CVehicle>(); //new CVehicle;
	*pVeh = *m_vVehicles.at(m_iCurVehicle);
	m_iCurVehicle++;
	return pVeh;
}

size_t CVehicleTrafficFile::getNoDays()
{
	return m_NoDays;
}

size_t CVehicleTrafficFile::getNoLanes()
{
	return m_NoLanes;
}

size_t CVehicleTrafficFile::getNoDirn()
{
	return m_NoDirn;
}

size_t CVehicleTrafficFile::getNoLanesDir1()
{
	return m_NoLanesDir1;
}

size_t CVehicleTrafficFile::getNoLanesDir2()
{
	return m_NoLanesDir2;
}

size_t CVehicleTrafficFile::getNoVehicles()
{
	return m_NoVehs;
}

double CVehicleTrafficFile::getStartTime()
{
	// make the start time the start of the first day of the sim
	return (double)((int)(m_Starttime/(3600.0*24))*3600.0*24);
}

double CVehicleTrafficFile::getEndTime()
{
	return m_Endtime;
}

