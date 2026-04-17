/**
 * @file VehicleGenGrave.h
 * @brief Interface for the CVehicleGenGrave class — the Grave model of French traffic.
 */

#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataGrave.h"

/**
 * @brief Vehicle generator implementing Sam Grave's model of French truck traffic.
 *
 * The Grave model samples truck geometry from multi-modal normal
 * distributions conditional on the number of axles and a vehicle class
 * index. Trucks with 2 or 3 axles are generated via GenerateTruck23(),
 * trucks with 4 or 5 axles via GenerateTruck45(); in both cases common
 * properties (length, track width, speed) come from GenerateCommonProps().
 *
 * This model was developed by Sam Grave for the French WIM data used
 * in calibrating Eurocode EN 1991-2 traffic load models.
 *
 * @see CVehicleGenerator
 * @see CVehModelDataGrave
 */
class CVehicleGenGrave : public CVehicleGenerator
{
public:
	/**
	 * @brief Construct with a Grave-type vehicle model data source.
	 * @param[in] pVMD Model data holding per-class axle weight and spacing distributions.
	 */
	CVehicleGenGrave(CVehModelDataGrave_sp pVMD);
	~CVehicleGenGrave();

protected:
	/**
	 * @brief Populate @p pVeh with a sampled truck.
	 *
	 * Samples the number of axles from the class distribution, then
	 * dispatches to GenerateTruck23() or GenerateTruck45() based on
	 * whether the truck has 2–3 or 4–5 axles.
	 */
	virtual void GenerateVehicle(CVehicle_sp pVeh);

	/**
	 * @brief Sample a vehicle-class index from the class distribution.
	 */
	virtual size_t GenVehClass();

private:
	/**
	 * @brief Generate a 2- or 3-axle truck.
	 * @param[in,out] pVeh   Vehicle to populate.
	 * @param[in]     nAxles Number of axles (2 or 3).
	 */
	void GenerateTruck23(CVehicle_sp pVeh, size_t nAxles);

	/**
	 * @brief Generate a 4- or 5-axle truck.
	 * @param[in,out] pVeh   Vehicle to populate.
	 * @param[in]     nAxles Number of axles (4 or 5).
	 */
	void GenerateTruck45(CVehicle_sp pVeh, size_t nAxles);

	/**
	 * @brief Populate length, track width, and other properties shared
	 *        across the 23- and 45-axle paths.
	 *
	 * @param[in,out] pVeh   Vehicle to populate.
	 * @param[in]     nAxles Number of axles.
	 */
	void GenerateCommonProps(CVehicle_sp pVeh, size_t nAxles);

	CVehModelDataGrave_sp m_pVMD;  ///< Grave model parameters.
};
typedef std::shared_ptr<CVehicleGenGrave> CVehicleGenGrave_sp;  ///< Shared-pointer alias for CVehicleGenGrave.
