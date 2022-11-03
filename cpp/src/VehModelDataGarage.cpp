#include "VehModelDataGarage.h"


CVehModelDataGarage::CVehModelDataGarage(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc)
	: CVehicleModelData(config, eVM_Garage, pVC, lfc, 1) // MAGIC NUMBER - truck class count
	, m_NoVehicles(0)
{
	// MAGIC NUMBER for now
	m_KernalGVW = Normal(1.0,0.08); // Mean and COV
	m_KernalAW = Normal(1.0, 0.05);
	m_KernalAS = Normal(1.0, 0.02);

	ReadDataIn();
}


CVehModelDataGarage::~CVehModelDataGarage()
{
}

void CVehModelDataGarage::ReadDataIn()
{
	readGarage();
	readKernels();
}

void CVehModelDataGarage::readGarage()
{
	CVehicleTrafficFile TrafficFile(m_pVehClassification, false, false, 0.0);
	std::cout << "Reading traffic garage file..." << std::endl;
	std::filesystem::path file = m_Config.Read.GARAGE_FILE;
	TrafficFile.Read(file.string(), m_Config.Read.FILE_FORMAT);

	assignGarage(TrafficFile.getVehicles());
}

void CVehModelDataGarage::assignGarage(std::vector<CVehicle_sp> vVehicles)
{
	m_NoVehicles = vVehicles.size();
	if (m_NoVehicles == 0)
		std::cout << "****ERROR: no vehicles in traffic garage file" << std::endl;

	m_vVehicles = vVehicles;
}

void CVehModelDataGarage::readKernels()
{
	std::filesystem::path file = m_Config.Read.KERNEL_FILE;
	
	if (!m_CSV.OpenFile(file.string(), ","))
		std::cerr << "***WARNING: Kernel file " 
				  << std::filesystem::weakly_canonical(file) 
				  << " could not be opened, using defaults" << std::endl;
	else
	{
		std::string line;
		
		m_CSV.getline(line);
		m_KernalGVW.Mean = m_CSV.stringToDouble(m_CSV.getfield(0));
		m_KernalGVW.StdDev = m_CSV.stringToDouble(m_CSV.getfield(1));
		
		m_CSV.getline(line);
		m_KernalAW.Mean = m_CSV.stringToDouble(m_CSV.getfield(0));
		m_KernalAW.StdDev = m_CSV.stringToDouble(m_CSV.getfield(1));
		
		m_CSV.getline(line);
		m_KernalAS.Mean = m_CSV.stringToDouble(m_CSV.getfield(0));
		m_KernalAS.StdDev = m_CSV.stringToDouble(m_CSV.getfield(1));
	}
}

CVehicle_sp CVehModelDataGarage::getVehicle(size_t i)
{
	if (i < m_NoVehicles)
		return m_vVehicles.at(i);
	else
		return nullptr;
}

void CVehModelDataGarage::getKernals(Normal& GVW, Normal& AW, Normal& AS)
{
	GVW = m_KernalGVW;
	AW = m_KernalAW;
	AS = m_KernalAS;
}
