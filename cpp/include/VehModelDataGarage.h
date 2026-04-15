/**
 * @file VehModelDataGarage.h
 * @brief Interface for the CVehModelDataGarage class — vehicle-pool model data.
 */

#pragma once

#include "VehicleModelData.h"
#include "VehicleTrafficFile.h"

/**
 * @brief Vehicle-model data holding a pre-recorded pool of vehicles
 *        (a "garage") used by @ref CVehicleGenGarage.
 *
 * Loads a set of observed vehicles from the configuration's
 * @c GARAGE_FILE (or is populated programmatically via the vector
 * constructor from the PyBTLS Python layer). The pool is replayed in
 * order by the garage vehicle generator, optionally with small
 * randomisation parametrised by the GVW, axle-weight, and axle-spacing
 * kernels loaded from @c KERNEL_FILE.
 *
 * @see CVehicleGenGarage
 * @see CVehicleModelData
 */
class CVehModelDataGarage :	public CVehicleModelData
{
public:
	/**
	 * @brief File-backed constructor — loads vehicles and kernels from disk.
	 */
	CVehModelDataGarage(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);

	/**
	 * @brief Vector-backed constructor — accepts an externally-supplied pool.
	 *
	 * Used by the PyBTLS Python layer when vehicles are constructed in
	 * Python rather than parsed from a file.
	 *
	 * @param[in] config        Shared configuration.
	 * @param[in] pVC           Vehicle classifier.
	 * @param[in] lfc           Lane flow composition.
	 * @param[in] vVehicles     Pre-built vehicle pool.
	 * @param[in] vKernelParams Kernel parameters per randomised field.
	 */
	CVehModelDataGarage(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, std::vector<CVehicle_sp> vVehicles, std::vector<std::vector<double>> vKernelParams);
	virtual ~CVehModelDataGarage();

	/// @brief Read the garage and kernel input files.
	virtual void ReadDataIn();

	/// @brief Get the number of vehicles in the garage pool.
	size_t getGarageCount() const { return m_NoVehicles; };

	/**
	 * @brief Get the i-th vehicle from the garage pool.
	 * @param[in] i Zero-based pool index.
	 */
	CVehicle_sp getVehicle(size_t i);

	/**
	 * @brief Get the randomisation kernels for GVW, axle weight, and axle spacing.
	 * @param[out] GVW Kernel parameters for gross vehicle weight.
	 * @param[out] AW  Kernel parameters for axle weight.
	 * @param[out] AS  Kernel parameters for axle spacing.
	 */
	void getKernels(KernelParams& GVW, KernelParams& AW, KernelParams& AS);

private:
	/// @brief Parse the garage file into @c m_vVehicles.
	void readGarage();

	/// @brief Parse the kernel file into the @c m_Kernel* fields.
	void readKernels();

	/// @brief Assign the garage pool from a caller-supplied vector.
	void assignGarage(std::vector<CVehicle_sp> vVehicles);

	/// @brief Assign the kernel parameters from a caller-supplied vector-of-vectors.
	void assignKernels(std::vector<std::vector<double>> vKernelParams);

	std::vector<CVehicle_sp> m_vVehicles;  ///< Garage vehicle pool.
	size_t m_NoVehicles;                   ///< Number of vehicles in the pool.

	KernelParams m_KernelGVW;              ///< GVW randomisation kernel.
	KernelParams m_KernelAW;               ///< Axle-weight randomisation kernel.
	KernelParams m_KernelAS;               ///< Axle-spacing randomisation kernel.
};
typedef std::shared_ptr<CVehModelDataGarage> CVehModelDataGarage_sp;  ///< Shared-pointer alias for CVehModelDataGarage.
