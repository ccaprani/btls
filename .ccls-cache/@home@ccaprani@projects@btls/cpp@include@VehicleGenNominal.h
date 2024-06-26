#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataNominal.h"

class CVehicleGenNominal :	public CVehicleGenerator
{
public:
	CVehicleGenNominal(CVehModelDataNominal_sp pVMD);
	virtual ~CVehicleGenNominal();

protected:
	virtual void GenerateVehicle(CVehicle_sp pVeh);
	virtual size_t GenVehClass() { return 0; };

private:
	void randomize(CVehicle_sp pVeh);

	CVehModelDataNominal_sp m_pVMD;
	CVehicle_sp m_NominalVehicle;
	const double m_MinimumCOV;
};
typedef std::shared_ptr<CVehicleGenNominal> CVehicleGenNominal_sp;
