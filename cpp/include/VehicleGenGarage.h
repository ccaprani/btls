/**
 * @file VehicleGenGarage.h
 * @brief Interface for the CVehicleGenGarage class — draws vehicles from a stored pool.
 */

#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGarage.h"

/**
 * @brief Vehicle generator that draws from a pre-recorded pool of vehicles.
 *
 * CVehicleGenGarage ("garage" as in a stable of vehicles) replays
 * observed vehicles in a round-robin or index-based manner rather than
 * sampling from a parametric distribution. This is useful when the
 * input data is trusted to be representative and the simulation wants
 * to study load effects on a specific fleet, not a synthetic one.
 *
 * Each call to GenerateVehicle() copies the geometry and weights from
 * the next stored vehicle onto the target, with optional small
 * perturbation via the randomize() helper.
 *
 * @see CVehicleGenerator
 * @see CVehModelDataGarage
 */
class CVehicleGenGarage : public CVehicleGenerator
{
public:
	/**
	 * @brief Construct with a garage-type vehicle model data source.
	 * @param[in] pVMD Model data holding the pre-recorded vehicle pool.
	 */
	CVehicleGenGarage(CVehModelDataGarage_sp pVMD);
	virtual ~CVehicleGenGarage();

protected:
	/**
	 * @brief Populate @p pVeh from the next vehicle in the garage pool.
	 */
	virtual void GenerateVehicle(CVehicle_sp pVeh);

	/// @brief Garage mode does not distinguish classes; always returns 0.
	virtual size_t GenVehClass() { return 0; };

private:
	/// @brief Apply small random perturbations to @p pVeh's weights and spacings.
	void randomize(CVehicle_sp pVeh);

	CVehModelDataGarage_sp m_pVMD;  ///< Pre-recorded vehicle pool.
	size_t m_GarageCount;           ///< Running cursor into the garage pool.
};
typedef std::shared_ptr<CVehicleGenGarage> CVehicleGenGarage_sp;  ///< Shared-pointer alias for CVehicleGenGarage.
