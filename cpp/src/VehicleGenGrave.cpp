#include "VehicleGenGrave.h"


//////////////////// CVehicleGenGrave ////////////////////////

CVehicleGenGrave::CVehicleGenGrave(CVehModelDataGrave_sp pVMD)
	: CVehicleGenerator(eVM_Grave, pVMD)
{
	m_pVMD = std::dynamic_pointer_cast<CVehModelDataGrave>(m_pVehModelData);

	m_pVehClassification = m_pVMD->getVehClassification();
}

CVehicleGenGrave::~CVehicleGenGrave()
{

}

void CVehicleGenGrave::GenerateVehicle(CVehicle_sp pVeh)
{
	CVehicleGenerator::GenerateVehicle(pVeh);

	// determine type of vehicle
	if (NextVehicleIsCar())
		GenerateCar(pVeh);
	else
	{
		size_t nAxles = GenVehClass();
		if (nAxles > 3)
			GenerateTruck45(pVeh, nAxles);
		else
			GenerateTruck23(pVeh, nAxles);
	}
	m_pVehClassification->setClassification(pVeh);
}

size_t CVehicleGenGrave::GenVehClass()
{
	double prop = m_RNG.GenerateUniform();
	vec vCP = m_pVMD->getComposition(m_CurHour);

	int nAxles = 1;
	double limit = 0.0;

	while (limit < prop)
	{
		nAxles++;
		limit += vCP.at(nAxles - 2); // /100; *** NOTE: new LaneFlow format has already /100
	}

	return nAxles;
}

void CVehicleGenGrave::GenerateTruck23(CVehicle_sp pVeh, size_t nAxles)
{
	GenerateCommonProps(pVeh, nAxles);

	// Generate Axle Weights
	std::vector<double> vAW(nAxles, 0.0);
	for (size_t i = 0; i < nAxles; ++i)
	{
		double val = -1.0;
		while (val < 15 || val > 500)
			val = m_RNG.GenerateMultiModalNormal(m_pVMD->GetAxleWeightDist(nAxles, i));
		vAW[i] = val*0.981;	// kg/100 to kN
	}

	double sumAW = SumVector(vAW);
	double GVW = pVeh->getGVW();	// since it's already set
	ScaleVector(vAW, GVW / sumAW);	// scale AWs to get correct GVW

	// And assign to vehicle
	for (size_t i = 0; i < nAxles; ++i)
		pVeh->setAW(i, vAW[i]);

}

void CVehicleGenGrave::GenerateTruck45(CVehicle_sp pVeh, size_t nAxles)
{
	GenerateCommonProps(pVeh, nAxles);

	// Generate Axle Weights
	double GVW = pVeh->getGVW();	// since it's already set
	int iRange = int((GVW - 25.0) / 50.0) + 1;	// iRange - index of truck weight in 50 kN intervals
	std::vector<double> vAWdist = m_pVMD->GetGVWRange(nAxles, iRange);
	std::vector<double> vAW(5, 0.0);

	for (int i = 0; i < 3; ++i)
	{
		double val = -1.0;
		while (val < 15 || val > 500)
			val = m_RNG.GenerateNormal(vAWdist[i], vAWdist[i + 3]);
		vAW[i] = val*0.981;	// kg/100 to kN
	}

	// scale AWs to get correct GVW
	double sumAW = SumVector(vAW);
	ScaleVector(vAW, GVW / sumAW);

	// split the tandem/tridem weight
	double tdemW = vAW[2];
	double indivAW = nAxles == 4 ? tdemW / 2 : tdemW / 3;
	for (size_t i = 2; i < nAxles; ++i)
		vAW[i] = indivAW;

	// And assign to vehicle
	for (size_t i = 0; i < nAxles; ++i)
		pVeh->setAW(i, vAW[i]);
}

void CVehicleGenGrave::GenerateCommonProps(CVehicle_sp pVeh, size_t nAxles)
{
	pVeh->setNoAxles(nAxles);

	// Generate GVW, AS, length properties
	double GVW = -1.0;
	while (GVW < 35 || GVW > 1000)
		GVW = m_RNG.GenerateMultiModalNormal(m_pVMD->GetGVW(m_CurDirection, nAxles));
	GVW = GVW*0.981; // kg/100 to kN

	// Gen axle spacings
	std::vector<double> vAS(nAxles, 0.0);
	for (size_t i = 0; i < nAxles - 1; ++i)	// no last-axle spacing
	{
		double val = -1.0;
		while (val < 0.5 || val > 200)
			val = m_RNG.GenerateMultiModalNormal(m_pVMD->GetSpacingDist(nAxles, i));
		vAS[i] = val / 10; // dm to m
	}
	double length = SumVector(vAS);

	// Gen axle track widths
	std::vector<double> vATW(nAxles, 0.0);
	// do first axle and see if to be constant for all other axles
	double val = -1.0;
	while (val < 120.0 || val > 260.0)	// physical bounds
		val = m_RNG.GenerateMultiModalNormal(m_pVMD->GetTrackWidthDist(nAxles, 0));
	vATW[0] = val / 100; // cm to m
	for (size_t i = 1; i < nAxles; ++i)
	{
		CMultiModalNormal tmn = m_pVMD->GetTrackWidthDist(nAxles, i);
		if (tmn.m_vModes[0].Mean > 1e-6)	// only generate if a new value is required
		{
			val = -1.0;
			while (val < 120.0 || val > 260.0)	// physical bounds
				val = m_RNG.GenerateMultiModalNormal(m_pVMD->GetTrackWidthDist(nAxles, i));
		}
		vATW[i] = val / 100; // cm to m
	}

	// assign these new properties to vehicle
	pVeh->setGVW(GVW);
	for (size_t i = 0; i < nAxles; ++i)
	{
		pVeh->setAS(i, vAS[i]);
		pVeh->setAT(i, vATW[i]);
	}
	pVeh->setLength(length);
}

