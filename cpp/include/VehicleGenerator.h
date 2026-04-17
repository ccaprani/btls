/**
 * @file VehicleGenerator.h
 * @brief Interface for the CVehicleGenerator class — base vehicle generator.
 */

#pragma once

#include <memory>
#include <vector>
#include "Generator.h"
#include "Vehicle.h"
#include "VehicleModelData.h"
#include "FlowModelData.h"

/**
 * @brief Abstract base class for vehicle generators.
 *
 * A CVehicleGenerator samples vehicle geometry — axle count, axle
 * weights, axle spacings, overall length, and track widths — from a
 * vehicle model. Concrete subclasses correspond to different modelling
 * philosophies:
 *
 * - @ref CVehicleGenGrave: Sam Grave's French-traffic model used in
 *   Eurocode development. Samples axle weights as multi-modal normals
 *   conditional on vehicle class.
 * - @ref CVehicleGenNominal: produces a single user-supplied "nominal"
 *   vehicle with small random perturbations.
 * - @ref CVehicleGenGarage: draws vehicles by index from a pre-recorded
 *   "garage" of observed vehicles.
 *
 * The base class also handles car generation separately from truck
 * generation via GenerateCar(), driven by a per-hour car-percentage
 * vector.
 *
 * @see CGenerator
 * @see CVehicleGenGrave
 * @see CVehicleGenNominal
 * @see CVehicleGenGarage
 */
class CVehicleGenerator : public CGenerator
{
public:
	/**
	 * @brief Construct with a vehicle model tag and shared model data.
	 * @param[in] vm  Vehicle model selector (enum EVehicleModel).
	 * @param[in] pVMD Vehicle model data (weights, spacings, classes).
	 */
	CVehicleGenerator(EVehicleModel vm, CVehicleModelData_sp pVMD);
	virtual ~CVehicleGenerator();

	/**
	 * @brief Sample one vehicle for the current hour.
	 *
	 * Chooses between car and truck generation based on the car-percentage
	 * for @p iHour, then dispatches to GenerateCar() or the subclass's
	 * GenerateVehicle() override.
	 *
	 * @param[in] iHour Current hour of day (0–23).
	 * @return Newly-sampled vehicle.
	 */
	CVehicle_sp Generate(size_t iHour);

	/**
	 * @brief Refresh the generator's view of the flow model data.
	 *
	 * Called by the lane when the flow model parameters change (e.g. at
	 * block boundaries).
	 *
	 * @param[in] pFMD Flow model data.
	 */
	virtual void update(CFlowModelData_sp pFMD);

protected:
	/**
	 * @brief Populate @p pVeh with truck geometry and weights.
	 *
	 * Subclasses implement this to supply the truck-generation strategy
	 * specific to their model.
	 */
	virtual void GenerateVehicle(CVehicle_sp pVeh);

	/**
	 * @brief Sample a truck-class index from the class distribution.
	 * @return Zero-based class index.
	 */
	virtual size_t GenVehClass() = 0;

	/**
	 * @brief Populate @p pVeh as a car (lighter, shorter, fewer axles).
	 */
	void GenerateCar(CVehicle_sp pVeh);

	/**
	 * @brief Decide whether the next generated vehicle should be a car
	 *        based on the current hour's car percentage.
	 */
	bool NextVehicleIsCar();

	EVehicleModel m_VehModel;                    ///< Active vehicle-model selector.
	CVehicleModelData_sp m_pVehModelData;        ///< Shared vehicle model data.
	bool m_bModelHasCars;                        ///< True if this model includes cars in its mix.

	vec m_vCarPercent;                           ///< Per-hour car percentage (24-element vector).

	size_t m_CurLane;                            ///< Lane of the current generation target.
	size_t m_CurDirection;                       ///< Direction of the current generation target.
	double m_Time;                               ///< Simulation time (seconds) of the current generation.
	size_t m_CurHour;                            ///< Current hour-of-day bucket (0–23).

	CVehicleClassification_sp m_pVehClassification;  ///< Vehicle classifier shared across generators.

	double (CDistribution::*m_pKernelGenerator)(double, double);  ///< Cached kernel sampler (avoids per-call switch).

private:
	/// @brief Cache the kernel sampler function pointer based on the current model.
	void SetKernelGenerator();

};
typedef std::shared_ptr<CVehicleGenerator> CVehicleGenerator_sp;  ///< Shared-pointer alias for CVehicleGenerator.
