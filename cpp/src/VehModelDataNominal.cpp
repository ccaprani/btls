#include "VehModelDataNominal.h"


CVehModelDataNominal::CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc)
	: CVehicleModelData(config, eVM_Constant, pVC, lfc, 1) // MAGIC NUMBER - truck class count
	, m_pNominalVehicle(nullptr)
{
	// MAGIC NUMBER = bias of 1.0
	m_KernelAS 	= KernelParams(1.0,0.05);
	m_KernelAW 	= KernelParams(1.0,0.05);

	makeNominalVehicle();
	ReadDataIn();
}

CVehModelDataNominal::CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, CVehicle_sp pVeh, std::vector<double> vCOV)
	: CVehicleModelData(config, eVM_Constant, pVC, lfc, 1) // MAGIC NUMBER - truck class count
	, m_pNominalVehicle(nullptr)
{
	// MAGIC NUMBER = bias of 1.0
	m_KernelAS 	= KernelParams(1.0,0.05);
	m_KernelAW 	= KernelParams(1.0,0.05);

	m_KernelAS.Scale = vCOV.at(0);
	m_KernelAW.Scale = vCOV.at(1);

	assignNominalVehicle(pVeh);
}

CVehModelDataNominal::~CVehModelDataNominal()
{
}

void CVehModelDataNominal::ReadDataIn()
{
	readNominalVehicle();
}

void CVehModelDataNominal::readNominalVehicle()
{
	std::filesystem::path file = m_Config.Read.NOMINAL_FILE;
	if (!m_CSV.OpenFile(file.string(), ","))
	{
		std::cerr << "***WARNING: Constant Vehicle file " 
				  << std::filesystem::weakly_canonical(file) 
				  << " could not be opened, using defaults" << std::endl;

		//Constant vehicle defaults
		m_pNominalVehicle->setGVW(460);
		m_pNominalVehicle->setNoAxles(6);
		int i = 0;	m_pNominalVehicle->setAS(i, 3.5); m_pNominalVehicle->setAW(i, 70); m_pNominalVehicle->setAT(i, 2.4);
		i++;		m_pNominalVehicle->setAS(i, 2.0); m_pNominalVehicle->setAW(i, 60); m_pNominalVehicle->setAT(i, 2.4);
		i++;		m_pNominalVehicle->setAS(i, 6.0); m_pNominalVehicle->setAW(i, 60); m_pNominalVehicle->setAT(i, 2.4);
		i++;		m_pNominalVehicle->setAS(i, 1.2); m_pNominalVehicle->setAW(i, 90); m_pNominalVehicle->setAT(i, 2.4);
		i++;		m_pNominalVehicle->setAS(i, 1.2); m_pNominalVehicle->setAW(i, 90); m_pNominalVehicle->setAT(i, 2.4);
		i++;		m_pNominalVehicle->setAS(i, 0.0); m_pNominalVehicle->setAW(i, 90); m_pNominalVehicle->setAT(i, 2.4);
		m_pNominalVehicle->setLength(13.9);
		m_pNominalVehicle->setHead(1001);
	}
	else
	{
		std::string line;
		
		// First line has the COVs (can be zero)
		m_CSV.getline(line);
		m_KernelAS.Scale = m_CSV.stringToDouble(m_CSV.getfield(0));
		m_KernelAW.Scale = m_CSV.stringToDouble(m_CSV.getfield(1));

		// Following lines have the axle spacings and weights
		unsigned int i = 0;
		std::vector<double> vAS;
		std::vector<double> vAW;
		while(m_CSV.getline(line))
		{
			vAS.push_back(m_CSV.stringToDouble(m_CSV.getfield(0)));
			vAW.push_back(m_CSV.stringToDouble(m_CSV.getfield(1)));
			i++;
		}
		m_pNominalVehicle->setNoAxles(i);
		double length = 0.0;
		double gvw = 0.0;
		for(unsigned int j = 0; j< i; j++)
		{
			length += vAS[j];
			m_pNominalVehicle->setAS(j, vAS[j]);
			gvw += vAW[j];
			m_pNominalVehicle->setAW(j, vAW[j]); 
			m_pNominalVehicle->setAT(j, 2.4);
		}
		m_pNominalVehicle->setLength(length);
		m_pNominalVehicle->setGVW(gvw);
		m_pNominalVehicle->setHead(1001);
	}
}

void CVehModelDataNominal::assignNominalVehicle(CVehicle_sp pVeh)
{
	pVeh->setHead(1001);
	m_pNominalVehicle = pVeh;
}

void CVehModelDataNominal::makeNominalVehicle()
{
	m_pNominalVehicle = std::make_shared<CVehicle>();
}

void CVehModelDataNominal::getKernels(KernelParams& AS, KernelParams& AW)
{
	AW = m_KernelAW;
	AS = m_KernelAS;
}