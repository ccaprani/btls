/**
 * @file VehModelDataNominal.h
 * @brief Interface for the CVehModelDataNominal class — nominal-vehicle model data.
 */

#pragma once
#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

/**
 * @brief Vehicle-model data for the nominal-vehicle model.
 *
 * Holds a single user-supplied nominal vehicle along with the kernel
 * parameters used to apply small random perturbations to its axle
 * weights and axle spacings on each generation. Used by
 * @ref CVehicleGenNominal when the simulation studies the load effect
 * of one specific vehicle type (e.g. a heavy permit load) rather than
 * a representative fleet.
 *
 * The vehicle can be loaded from @c NOMINAL_FILE or supplied
 * programmatically through the vector constructor from the PyBTLS
 * Python layer.
 *
 * @see CVehicleGenNominal
 * @see CVehicleModelData
 */
class CVehModelDataNominal : public CVehicleModelData
{
public:
    /**
     * @brief File-backed constructor — reads the nominal vehicle from disk.
     */
    CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);

    /**
     * @brief Vector-backed constructor — accepts a caller-supplied vehicle and COV list.
     *
     * @param[in] config Shared configuration.
     * @param[in] pVC    Vehicle classifier.
     * @param[in] lfc    Lane flow composition.
     * @param[in] pVeh   Nominal vehicle.
     * @param[in] vCOV   Coefficient-of-variation per randomised field.
     */
    CVehModelDataNominal(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, CVehicle_sp pVeh, std::vector<double> vCOV);
	virtual ~CVehModelDataNominal();

	/// @brief Read the nominal vehicle and perturbation kernels from disk.
	virtual void ReadDataIn();

	/**
	 * @brief Get the randomisation kernels for axle weight and spacing.
	 * @param[out] AW Kernel parameters for axle weight.
	 * @param[out] AS Kernel parameters for axle spacing.
	 */
	void getKernels(KernelParams& AW, KernelParams& AS);

    /// @brief Get the nominal vehicle template (shared).
    CVehicle_sp getNominalVehicle() {return m_pNominalVehicle;};

private:
	/// @brief Parse the nominal vehicle file.
	void readNominalVehicle();

    /// @brief Assign the nominal vehicle from a caller-supplied pointer.
    void assignNominalVehicle(CVehicle_sp pVeh);

    /// @brief Build a default nominal vehicle programmatically.
    void makeNominalVehicle();

    CVehicle_sp m_pNominalVehicle;  ///< Nominal vehicle template cloned for every generation.

    KernelParams m_KernelAW;        ///< Axle-weight randomisation kernel.
    KernelParams m_KernelAS;        ///< Axle-spacing randomisation kernel.
};
typedef std::shared_ptr<CVehModelDataNominal> CVehModelDataNominal_sp;  ///< Shared-pointer alias for CVehModelDataNominal.
