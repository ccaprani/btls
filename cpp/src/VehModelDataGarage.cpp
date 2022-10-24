#include "VehModelDataGarage.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CVehModelDataGarage::CVehModelDataGarage(CVehicleClassification_sp pVC, CLaneFlowComposition lfc)
	: CVehicleModelData(eVM_Garage, pVC, lfc, 1) // MAGIC NUMBER - truck class count
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
	filesystem::path file = CConfigData::get().Read.GARAGE_FILE;
	extractGarage(file);
}

void CVehModelDataGarage::readGarage(std::string garage_file)
{
	filesystem::path file = garage_file;
	extractGarage(file);
}

void CVehModelDataGarage::extractGarage(filesystem::path file) {
	CVehicleTrafficFile TrafficFile(m_pVehClassification, false, false, 0.0);
	std::cout << "Reading traffic garage file..." << std::endl;
	TrafficFile.Read(file.string(), CConfigData::get().Read.FILE_FORMAT);

	m_NoVehicles = TrafficFile.getNoVehicles();
	if (m_NoVehicles == 0)
		std::cout << "****ERROR: no vehicles in traffic garage file" << std::endl;

	m_vVehicles = TrafficFile.getVehicles();
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
	filesystem::path file = CConfigData::get().Read.KERNEL_FILE;
	extractKernels(file);
}

void CVehModelDataGarage::readKernels(std::string kernel_file)
{
	filesystem::path file = kernel_file;
	extractKernels(file);
}

void CVehModelDataGarage::extractKernels(filesystem::path kernel_file) {
	if (!m_CSV.OpenFile(kernel_file.string(), ","))
		std::cerr << "***WARNING: Kernel file " 
				  << std::filesystem::weakly_canonical(kernel_file) 
				  << " could not be opened, using defaults" << endl;
	else
	{
		string line;
		
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