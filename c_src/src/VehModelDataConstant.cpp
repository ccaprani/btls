#include "VehModelDataConstant.h"
#include "ConfigData.h"

//extern CConfigData g_ConfigData;

CVehModelDataConstant::CVehModelDataConstant(CVehicleClassification_sp pVC, CLaneFlowComposition lfc)
	: CVehicleModelData(eVM_Constant, pVC, lfc, 1) // MAGIC NUMBER - truck class count
	, m_pNominalVehicle(nullptr), m_CoV_AS(0.0), m_CoV_AW(0.0)
{
    makeNominalVehicle();
	ReadDataIn();    
}


CVehModelDataConstant::~CVehModelDataConstant()
{
}

void CVehModelDataConstant::ReadDataIn()
{
	readConstant();
}

void CVehModelDataConstant::readConstant()
{
    filesystem::path file = CConfigData::get().Read.CONSTANT_FILE;
	if (!m_CSV.OpenFile(file, ","))
    {
		std::cerr << "***WARNING: Constant Vehicle file could not be opened, using defaults" << endl;

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
    }
	else
	{
		string line;
		
        // First line has the COVs (can be zero)
		m_CSV.getline(line);
		m_CoV_AS = m_CSV.stringToDouble(m_CSV.getfield(0));
		m_CoV_AW = m_CSV.stringToDouble(m_CSV.getfield(1));

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
    }
}

void CVehModelDataConstant::makeNominalVehicle()
{
    m_pNominalVehicle = std::make_shared<CVehicle>(); //new CVehicle;

    // assign general properties
    m_pNominalVehicle->setLaneEccentricity(0.0);
    m_pNominalVehicle->setTrns(0.0); // m 0 for generated vehicles
    m_pNominalVehicle->setHead(1001);
}

void CVehModelDataConstant::getCoVs(double& CoV_AS, double& CoV_AW)
{
    CoV_AS = m_CoV_AS;
    CoV_AW = m_CoV_AW;
}