#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <math.h>
#include "Event.h"
#include "Effect.h"
#include "Vehicle.h"

class COutputManagerBase
{
public:
	COutputManagerBase(std::string filestem);
	virtual ~COutputManagerBase(void);

	virtual void Update(CEvent Ev) = 0;
	void Finish();
	//virtual void setOutFile(double BridgeLength);
	virtual void Initialize(double BridgeLength, std::vector<double> vThreshold, double SimStartTime) {};
	virtual void Initialize(double BridgeLength, size_t nLE) {};

protected:
	virtual void	WriteVehicleFiles(){};
	virtual void	WriteSummaryFiles() = 0;
	virtual void	CheckBuffer(bool bForceOutput) = 0;
	virtual void	OpenVehicleFiles(){};
	
	virtual void	WriteBuffer();
	
	void OpenSummaryFiles();
	void OpenVehicleFile(size_t i);
	
	double	m_BridgeLength;
	const std::string m_FileStem;

	double m_SimStartTime;

	size_t m_NoLoadEffects;
	std::ofstream m_OutFile;
	std::vector<std::string>	m_vOutFiles;
	std::vector<std::string>	m_vSummaryFiles;

	bool			WRITE_VEHICLES;
	bool			WRITE_SUMMARY;
	unsigned int	WRITE_BUFFER_SIZE;

	// define template function in header file
	// See http://www.parashift.com/c++-faq-lite/templates.html#faq-35.12
	template <typename T>
	std::string to_string(T const& value)
	{
		std::stringstream sstr;
		sstr << value;
		return sstr.str();
	}

	template <typename T>
	std::string to_string(T const& value, int nDigits)
	{
		std::stringstream sstr;
		sstr << std::fixed << std::setprecision(nDigits) << value;
		return sstr.str();
	}
};

