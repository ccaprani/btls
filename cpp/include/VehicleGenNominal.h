/**
 * @file VehicleGenNominal.h
 * @brief Interface for the CVehicleGenNominal class — nominal-vehicle generator with optional perturbation.
 */

#pragma once
#include "VehicleGenerator.h"
#include "VehModelDataNominal.h"

/**
 * @brief Vehicle generator that produces a single user-supplied nominal
 *        vehicle, optionally perturbed by small random variations.
 *
 * CVehicleGenNominal is used when the simulation wants to study the
 * load effect of one known vehicle — or a tightly-constrained family
 * around it — rather than a representative fleet. A user-supplied
 * @c m_NominalVehicle carries the baseline geometry and weights; each
 * call to GenerateVehicle() copies it onto the target and then optionally
 * applies small random perturbations via randomize(), bounded below
 * by @c m_MinimumCOV (minimum coefficient of variation).
 *
 * @see CVehicleGenerator
 * @see CVehModelDataNominal
 */
class CVehicleGenNominal : public CVehicleGenerator
{
public:
	/**
	 * @brief Construct with a nominal-type vehicle model data source.
	 * @param[in] pVMD Model data holding the baseline vehicle and perturbation limits.
	 */
	CVehicleGenNominal(CVehModelDataNominal_sp pVMD);
	virtual ~CVehicleGenNominal();

protected:
	/**
	 * @brief Populate @p pVeh with a copy of the nominal vehicle, then randomize().
	 */
	virtual void GenerateVehicle(CVehicle_sp pVeh);

	/// @brief Nominal mode does not distinguish classes; always returns 0.
	virtual size_t GenVehClass() { return 0; };

private:
	/// @brief Apply small random perturbations to @p pVeh's weights and spacings.
	void randomize(CVehicle_sp pVeh);

	CVehModelDataNominal_sp m_pVMD;        ///< Nominal model parameters.
	CVehicle_sp m_NominalVehicle;          ///< Baseline vehicle cloned on every generation.
	const double m_MinimumCOV;             ///< Lower bound on the coefficient of variation used when perturbing.
};
typedef std::shared_ptr<CVehicleGenNominal> CVehicleGenNominal_sp;  ///< Shared-pointer alias for CVehicleGenNominal.
