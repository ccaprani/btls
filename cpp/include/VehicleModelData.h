/**
 * @file VehicleModelData.h
 * @brief Interface for the CVehicleModelData class — base for vehicle-model data containers.
 */

#pragma once
#include "ModelData.h"
#include "VehicleClassification.h"
#include "LaneFlowComposition.h"

/**
 * @brief Intermediate base class for vehicle-model data containers.
 *
 * CVehicleModelData carries the fields that every vehicle model needs:
 * the model selector, the classification scheme, the number of truck
 * classes, the per-lane class composition matrix, and the lane
 * eccentricity standard deviation. Concrete subclasses —
 * @ref CVehModelDataGrave, @ref CVehModelDataGarage,
 * @ref CVehModelDataNominal — layer model-specific distribution data
 * on top.
 *
 * @see CModelData
 * @see CVehicleGenerator
 */
class CVehicleModelData : public CModelData
{
public:
	/**
	 * @brief Construct a vehicle-model-data container.
	 *
	 * @param[in] config  Shared configuration block.
	 * @param[in] vm      Vehicle-model selector.
	 * @param[in] pVC     Vehicle classifier.
	 * @param[in] lfc     Lane flow composition (lane number, direction, flow rates).
	 * @param[in] nClass  Number of truck classes supported by this model.
	 */
	CVehicleModelData(CConfigDataCore& config, EVehicleModel vm, CVehicleClassification_sp pVC, CLaneFlowComposition lfc, const size_t nClass);
	virtual ~CVehicleModelData();

	/// @brief Get the model selector this container was constructed with.
	EVehicleModel getModel() const { return m_Model; };

	/// @brief Get the number of truck classes supported.
	size_t getTruckClassCount() const { return m_TruckClassCount; };

	/// @brief Get the vehicle classifier used by this model.
	CVehicleClassification_sp getVehClassification() const { return m_pVehClassification; };

	/// @brief Get the standard deviation of the lane eccentricity perturbation, in metres.
	double getLaneEccStd() const { return m_LaneEccStd; };

	/**
	 * @brief Get the truck-class composition vector for lane @p i.
	 * @param[in] i Zero-based lane index.
	 * @return Probability vector over truck classes summing to 1.0.
	 */
	vec getComposition(size_t i) const;

	/// @brief Get the kernel shape used for randomisation (normal or triangle).
	EKernelType getKernelType() const {return m_KernelType;};

protected:
	EVehicleModel m_Model;                       ///< Active model selector.
	const size_t m_TruckClassCount;              ///< Number of truck classes.

	CVehicleClassification_sp m_pVehClassification;  ///< Vehicle classifier.

	EKernelType m_KernelType;                    ///< Kernel shape (normal or triangle) used for randomisation.

	size_t m_NoLanes;                            ///< Number of lanes covered by this model.
	size_t m_CurDirection;                       ///< Direction this data instance is associated with.

	matrix m_mComposition;                       ///< Per-lane truck-class composition matrix.

	double m_LaneEccStd;                         ///< Standard deviation of lane eccentricity in metres.

private:
};
typedef std::shared_ptr<CVehicleModelData> CVehicleModelData_sp;  ///< Shared-pointer alias for CVehicleModelData.
