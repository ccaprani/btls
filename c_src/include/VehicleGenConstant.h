#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataConstant.h"

class CVehicleGenConstant :	public CVehicleGenerator
{
public:
	CVehicleGenConstant(CVehModelDataConstant_sp pVMD);
	virtual ~CVehicleGenConstant();

protected:
	virtual void GenerateVehicle(CVehicle_sp pVeh);
	virtual size_t GenVehClass() { return 0; };

private:
	void randomize(CVehicle_sp pVeh);

	CVehModelDataConstant_sp m_pVMD;
	const double m_MinimumCOV;
};
typedef std::shared_ptr<CVehicleGenConstant> CVehicleGenConstant_sp;
