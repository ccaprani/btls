/**
 * @file FlowModelData.h
 * @brief Interface for the CFlowModelData class and its four concrete subclasses.
 */

#pragma once
#include <memory>
#include <vector>
#include "ModelData.h"
#include "LaneFlowComposition.h"

/**
 * @brief Intermediate base class for flow-model data containers.
 *
 * CFlowModelData carries the shared per-block flow parameters used by
 * every headway model: total flow, truck flow, car percentage, speed
 * distribution, block size and block count. The four concrete
 * subclasses add model-specific data read from their respective input
 * files via ReadDataIn().
 *
 * Blocks are the daily time slices within which the flow rate is taken
 * to be constant. The default is 24 hourly blocks of 3600 s each; see
 * @ref CFlowGenerator for how the blocks drive the simulation cycle.
 *
 * @see CModelData
 * @see CFlowGenerator
 * @see CFlowModelDataHeDS
 * @see CFlowModelDataCongested
 * @see CFlowModelDataPoisson
 * @see CFlowModelDataConstant
 */
class CFlowModelData :	public CModelData
{
public:
	/**
	 * @brief Construct a flow-model-data container.
	 *
	 * @param[in] config Shared configuration block.
	 * @param[in] fm     Flow-model selector.
	 * @param[in] lfc    Lane flow composition.
	 * @param[in] bCars  If true, the model includes cars in its mix.
	 */
	CFlowModelData(CConfigDataCore& config, EFlowModel fm, CLaneFlowComposition lfc, const bool bCars);
	virtual ~CFlowModelData();

	/// @brief Get the per-block car percentage vector (24 entries for 24 hours).
	vec getCarPercent() const { return m_vCarPercent; };

	/**
	 * @brief Get the total and truck flow for block @p i.
	 * @param[in]  i         Block index (0..23 for hourly blocks).
	 * @param[out] totalFlow Total flow rate for the block (veh/h).
	 * @param[out] truckFlow Truck flow rate for the block (truck/h).
	 */
	void	getFlow(size_t i, double& totalFlow, double& truckFlow);

	/**
	 * @brief Get the speed distribution parameters for block @p i.
	 * @param[in]  i     Block index.
	 * @param[out] speed Normal distribution with mean and std dev in m/s.
	 */
	void	getSpeedParams(size_t i, Normal& speed);

	/**
	 * @brief Get the buffer-gap parameters (additional safety spacing).
	 * @param[out] bridge Bridge length reference in metres.
	 * @param[out] space  Additional safety space in metres.
	 * @param[out] time   Additional safety time in seconds.
	 */
	void	getGapLimits(double& bridge, double& space, double& time);

	/// @brief Get the flow-model selector.
	EFlowModel getModel() const { return m_Model; };

	/// @brief Return true if this model generates cars in addition to trucks.
	bool getModelHasCars() const { return m_bModelHasCars; };

	/**
	 * @brief Get the block size and count.
	 * @param[out] sz Block size in seconds.
	 * @param[out] n  Number of blocks in one cycle.
	 */
	void getBlockInfo(size_t& sz, size_t& n) const;

protected:
	EFlowModel m_Model;                 ///< Active flow-model selector.
	const bool m_bModelHasCars;         ///< True if the model mix includes cars.

	vec m_vTotalFlow;                   ///< Total flow rate per block (veh/h).
	vec m_vTruckFlow;                   ///< Truck flow rate per block (truck/h).
	vec m_vCarPercent;                  ///< Car percentage per block.
	std::vector<Normal> m_vSpeed;       ///< Speed distribution per block.

	size_t m_BlockSize;                 ///< Block size in seconds (typically 3600).
	size_t m_BlockCount;                ///< Number of blocks per cycle (typically 24).

	double m_SpeedMean;                 ///< Overall speed mean (for convenience).
	double m_SpeedStd;                  ///< Overall speed std dev (for convenience).

	const double m_BufferGapSpace;      ///< Additional safety space in metres.
	const double m_BufferGapTime;       ///< Additional safety time in seconds.

private:

};
typedef std::shared_ptr<CFlowModelData> CFlowModelData_sp;  ///< Shared-pointer alias for CFlowModelData.


/**
 * @brief Flow-model data for the Headway Distribution Statistics (HeDS) model.
 *
 * Loads an empirical cumulative-distribution table of inter-vehicle
 * gaps, indexed by flow rate, from the HeDS input file. The matrix
 * @c m_vHeDS holds one headway CDF per flow-rate row; the matching
 * @ref CFlowGenHeDS samples from the current-flow-rate row at each
 * invocation.
 *
 * @see CFlowGenHeDS
 */
class CFlowModelDataHeDS : public CFlowModelData
{
public:
	CFlowModelDataHeDS(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataHeDS();

	/// @brief Read the HeDS input file and populate @c m_vHeDS.
	virtual void ReadDataIn();

	/// @brief Get a copy of the HeDS headway matrix.
	matrix GetHeDS();

private:
	/// @brief Parse the HeDS input file into @c m_vHeDS.
	void ReadFile_HeDS();

	matrix m_vHeDS;  ///< Flow-rate indexed headway CDF table.
};
typedef std::shared_ptr<CFlowModelDataHeDS> CFlowModelDataHeDS_sp;  ///< Shared-pointer alias for CFlowModelDataHeDS.


/**
 * @brief Flow-model data for congested-flow conditions.
 *
 * Holds a constant speed and a normally-distributed inter-vehicle gap
 * (mean + standard deviation), representing queued or congested
 * traffic where every vehicle travels at approximately the same slow
 * speed and maintains a tight headway.
 *
 * @see CFlowGenCongested
 */
class CFlowModelDataCongested : public CFlowModelData
{
public:
	CFlowModelDataCongested(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataCongested();

	/// @brief Populate from the configuration's Traffic.CONGESTED_* fields.
	virtual void ReadDataIn();

	/// @brief Get the congested-mode constant speed in m/s.
	double getSpeed() const { return m_Speed; };

	/**
	 * @brief Get the gap distribution parameters.
	 * @param[out] mean Gap mean in seconds.
	 * @param[out] std  Gap standard deviation in seconds.
	 */
	void getGapParams(double& mean, double& std);

private:
	double m_Speed;    ///< Constant congested-mode speed in m/s.
	double m_GapMean;  ///< Mean inter-vehicle gap in seconds.
	double m_GapStd;   ///< Standard deviation of the inter-vehicle gap.
};
typedef std::shared_ptr<CFlowModelDataCongested> CFlowModelDataCongested_sp;  ///< Shared-pointer alias for CFlowModelDataCongested.


/**
 * @brief Flow-model data for Poisson (exponential-headway) arrivals.
 *
 * No data beyond what the intermediate base class carries — the
 * Poisson arrival rate is taken directly from @c m_vTruckFlow for
 * the current block.
 *
 * @see CFlowGenPoisson
 */
class CFlowModelDataPoisson : public CFlowModelData
{
public:
	CFlowModelDataPoisson(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataPoisson();

	/// @brief Read any Poisson-specific inputs (currently a no-op; flow rates come from the base).
	virtual void ReadDataIn();
};
typedef std::shared_ptr<CFlowModelDataPoisson> CFlowModelDataPoisson_sp;  ///< Shared-pointer alias for CFlowModelDataPoisson.


/**
 * @brief Flow-model data for deterministic (constant-gap, constant-speed) traffic.
 *
 * Used for testing and validation where stochastic variation would
 * confuse the result. The constant gap and speed come from the
 * configuration's Traffic.CONSTANT_* fields.
 *
 * @see CFlowGenConstant
 */
class CFlowModelDataConstant : public CFlowModelData
{
public:
	CFlowModelDataConstant(CConfigDataCore& config, CLaneFlowComposition lfc);
	virtual ~CFlowModelDataConstant();

	/// @brief Get the constant speed in m/s.
	double getConstSpeed();

	/// @brief Get the constant gap in seconds.
	double getConstGap();

	/// @brief Load constant gap and speed from the configuration.
	virtual void ReadDataIn();

private:
	double m_ConstSpeed;  ///< Constant speed in m/s.
	double m_ConstGap;    ///< Constant gap in seconds.
};
typedef std::shared_ptr<CFlowModelDataConstant> CFlowModelDataConstant_sp;  ///< Shared-pointer alias for CFlowModelDataConstant.
