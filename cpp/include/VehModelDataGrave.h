/**
 * @file VehModelDataGrave.h
 * @brief Interface for the CVehModelDataGrave class — Grave-model vehicle data.
 */

#pragma once
#include "VehicleModelData.h"
#include "MultiModalNormal.h"

/**
 * @brief Vehicle-model data for the Grave model of French truck traffic.
 *
 * CVehModelDataGrave holds the full set of conditional distributions
 * that @ref CVehicleGenGrave samples from: axle-spacing multi-modal
 * normals by truck class, axle-weight multi-modal normals for 2- and
 * 3-axle trucks, GVW-banded weight distributions for 4- and 5-axle
 * trucks, per-direction GVW distributions, per-truck track-width
 * distributions, and per-direction speed distributions.
 *
 * Data is loaded from four input files — axle-weight-23, axle-weight-45,
 * axle-spacing, and GVW — via the ReadFile_* helpers, or populated
 * programmatically through the Add* setters called from the PyBTLS
 * Python layer.
 *
 * @see CVehicleGenGrave
 * @see CVehicleModelData
 * @see CMultiModalNormal
 */
class CVehModelDataGrave : public CVehicleModelData
{
public:
	CVehModelDataGrave(CConfigDataCore& config, CVehicleClassification_sp pVC, CLaneFlowComposition lfc);
	~CVehModelDataGrave();

	/// @brief Read all Grave-model input files.
	virtual void ReadDataIn();

	/**
	 * @brief Get the mean/std parameters for a GVW band.
	 * @param[in] iTruck Truck bucket (0 = 4-axle, 1 = 5-axle).
	 * @param[in] iRange GVW band index.
	 * @return Flattened (mean, std) pairs for W1, W2, WT.
	 */
	std::vector<double>	GetGVWRange(size_t iTruck, size_t iRange);

	/// @brief Get the spacing distribution for one spacing slot of one truck type.
	CMultiModalNormal		GetSpacingDist(size_t iTruck, size_t iSpace);

	/// @brief Get the axle-weight distribution for one axle of one truck type.
	CMultiModalNormal		GetAxleWeightDist(size_t iTruck, size_t iAxle);

	/// @brief Get the axle track-width distribution for one axle of one truck type.
	CMultiModalNormal		GetTrackWidthDist(size_t iTruck, size_t iAxle);

	/// @brief Get the GVW distribution for one direction and truck type.
	CMultiModalNormal		GetGVW(size_t dir, size_t iTruck);

	/// @brief Load 2-axle truck spacing distributions.
	void Add2AxleSpacings(std::vector<CMultiModalNormal> vSpace);
	/// @brief Load 3-axle truck spacing distributions.
	void Add3AxleSpacings(std::vector<CMultiModalNormal> vSpace);
	/// @brief Load 4-axle truck spacing distributions.
	void Add4AxleSpacings(std::vector<CMultiModalNormal> vSpace);
	/// @brief Load 5-axle truck spacing distributions.
	void Add5AxleSpacings(std::vector<CMultiModalNormal> vSpace);

	/// @brief Load 2-axle track-width distributions.
	void Add2AxleTrackWidth(std::vector<CMultiModalNormal> vSpace);
	/// @brief Load 3-axle track-width distributions.
	void Add3AxleTrackWidth(std::vector<CMultiModalNormal> vSpace);
	/// @brief Load 4-axle track-width distributions.
	void Add4AxleTrackWidth(std::vector<CMultiModalNormal> vSpace);
	/// @brief Load 5-axle track-width distributions.
	void Add5AxleTrackWidth(std::vector<CMultiModalNormal> vSpace);

	/// @brief Load 2-axle per-axle weight distributions.
	void Add2AxleWeight(std::vector<CMultiModalNormal> vAxle);
	/// @brief Load 3-axle per-axle weight distributions.
	void Add3AxleWeight(std::vector<CMultiModalNormal> vAxle);
	/**
	 * @brief Load one GVW band for 4- or 5-axle trucks.
	 * @param[in] data   Flattened (mean, std) parameters.
	 * @param[in] iTruck Truck bucket (0 = 4-axle, 1 = 5-axle).
	 * @param[in] iRange GVW band index.
	 */
	void Add45AxleWeight(std::vector<double> data, std::size_t iTruck, std::size_t iRange);

	/**
	 * @brief Load the per-truck GVW distributions for one direction.
	 * @param[in] dir  Direction (1 or 2).
	 * @param[in] vGVW One GVW distribution per truck type.
	 */
	void AddGVW(int dir, std::vector<CMultiModalNormal> vGVW);

	/**
	 * @brief Load the per-direction speed distributions.
	 *
	 * Speed is kept here as part of the class modelling rather than in
	 * the flow-model data.
	 */
	void AddSpeed(std::vector<CMultiModalNormal> vSpeed);

	/// @brief Get the speed distribution for one direction.
	CMultiModalNormal			GetSpeed(std::size_t dir);

private:
	/// @brief Parse the 2- and 3-axle weight file.
	void ReadFile_AW23();
	/// @brief Parse the 4- and 5-axle weight file.
	void ReadFile_AW45();
	/// @brief Parse the axle spacing file.
	void ReadFile_AS();
	/// @brief Parse the GVW file.
	void ReadFile_GVW();
	/// @brief Parse the axle track-width file.
	void ReadFile_ATW();

	std::vector<CMultiModalNormal> m_v2AxleSpacings;  ///< 2-axle spacing distributions.
	std::vector<CMultiModalNormal> m_v3AxleSpacings;  ///< 3-axle spacing distributions.
	std::vector<CMultiModalNormal> m_v4AxleSpacings;  ///< 4-axle spacing distributions.
	std::vector<CMultiModalNormal> m_v5AxleSpacings;  ///< 5-axle spacing distributions.

	std::vector<CMultiModalNormal> m_v2AxleTrackWidth;  ///< 2-axle track-width distributions.
	std::vector<CMultiModalNormal> m_v3AxleTrackWidth;  ///< 3-axle track-width distributions.
	std::vector<CMultiModalNormal> m_v4AxleTrackWidth;  ///< 4-axle track-width distributions.
	std::vector<CMultiModalNormal> m_v5AxleTrackWidth;  ///< 5-axle track-width distributions.

	std::vector<CMultiModalNormal> m_v2AxleWeight;  ///< 2-axle per-axle weight distributions.
	std::vector<CMultiModalNormal> m_v3AxleWeight;  ///< 3-axle per-axle weight distributions.

	std::vector<CMultiModalNormal> m_vDir1GVW;   ///< Per-truck GVW distributions for direction 1.
	std::vector<CMultiModalNormal> m_vDir2GVW;   ///< Per-truck GVW distributions for direction 2.
	std::vector<CMultiModalNormal> m_vSpeed;     ///< Per-direction speed distributions.

	/// @brief Simple (mean, std) pair.
	struct Dist
	{
		double Mean;
		double StdDev;
	};

	/// @brief W1, W2 and total weight distributions for one GVW band.
	struct GVWRange
	{
		Dist W1;  ///< Front axle weight.
		Dist W2;  ///< Rear axle group weight.
		Dist WT;  ///< Total weight.
	};

	std::vector<GVWRange> m_v4AxleWeight;  ///< GVW bands for 4-axle trucks.
	std::vector<GVWRange> m_v5AxleWeight;  ///< GVW bands for 5-axle trucks.
};
typedef std::shared_ptr<CVehModelDataGrave> CVehModelDataGrave_sp;  ///< Shared-pointer alias for CVehModelDataGrave.
