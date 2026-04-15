/**
 * @file FlowGenerator.h
 * @brief Interface for the CFlowGenerator class and its concrete subclasses — headway and speed models.
 */

#pragma once
#include <memory>
#include "Generator.h"
#include "FlowModelData.h"
#include "Vehicle.h"

//class CFlowGenerator; typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_sp;

/**
 * @brief Abstract base class for headway and speed generators.
 *
 * A CFlowGenerator produces the inter-vehicle gap (in seconds) and
 * speed (in metres per second) for the next vehicle on a lane. It
 * decouples the *when* of vehicle arrivals from the *what* (handled by
 * @ref CVehicleGenerator).
 *
 * The base class handles the shared bookkeeping:
 *
 * - Hourly **blocks**: @c m_BlockSize (typically 3600 s) defines one
 *   block, @c m_BlockCount (typically 24) is the number of blocks in
 *   a day. The current block index is advanced in updateBlock(),
 *   which is what gives the simulation a natural daily seam.
 * - **Minimum gap**: @c m_MinGap enforces a physically realistic
 *   spacing between vehicles, set in setMinGap() from the previous
 *   and next vehicle velocities and lengths.
 * - **Bridge length**: @c m_MaxBridgeLength lets the generator reason
 *   about multi-vehicle interactions on long spans.
 *
 * Concrete subclasses implement GenerateGap() and GenerateSpeed()
 * corresponding to different headway models:
 *
 * - @ref CFlowGenHeDS: Headway Distribution Statistics model
 *   (empirical per-flow-rate headway distributions).
 * - @ref CFlowGenCongested: congested flow with a normally-distributed
 *   gap and constant speed.
 * - @ref CFlowGenPoisson: Poisson arrivals (exponential gaps) with a
 *   normal speed distribution.
 * - @ref CFlowGenConstant: deterministic headway and speed, used for
 *   testing and validation.
 *
 * @see CGenerator
 * @see CFlowModelData
 * @see CLaneGenTraffic
 */
class CFlowGenerator : public CGenerator
{
public:
	/**
	 * @brief Construct a flow generator with the given model data and model tag.
	 * @param[in] pFDM Flow model data shared by all lanes using this model.
	 * @param[in] fm   Flow model selector (enum EFlowModel).
	 */
	CFlowGenerator(CFlowModelData_sp pFDM, EFlowModel fm);
	virtual ~CFlowGenerator();

	/**
	 * @brief Generate the next vehicle's gap and speed.
	 *
	 * Calls the subclass's GenerateGap() and GenerateSpeed() hooks,
	 * clamping the gap to @c m_MinGap. Returns the combined gap as a
	 * scalar for the lane to add to the current arrival time.
	 *
	 * @return Gap in seconds.
	 */
	double Generate();

	/**
	 * @brief Refresh the minimum-gap constraint from the previous and next vehicles.
	 *
	 * Called by the lane before each Generate() so the generator can
	 * honour car-following: the next vehicle's speed and length plus
	 * the previous vehicle's length determine the physical minimum
	 * headway.
	 *
	 * @param[in] time     Simulation time in seconds.
	 * @param[in] pPrevVeh Most recently emitted vehicle.
	 * @param[in] pNextVeh Next vehicle about to be emitted.
	 */
	virtual void prepareNextGen(double time, CVehicle_sp pPrevVeh, CVehicle_sp pNextVeh);

	/**
	 * @brief Record the maximum bridge length for minimum-gap reasoning.
	 * @param[in] length Bridge length in metres.
	 */
	void setMaxBridgeLength(double length);

	/// @brief Get the current hour-of-day block index (0-based, within one block cycle).
	size_t getCurBlock() {return m_CurBlock;};

protected:
	/// @brief Sample a gap from the subclass-specific headway model.
	virtual double GenerateGap() = 0;

	/// @brief Sample a speed from the subclass-specific speed model.
	virtual double GenerateSpeed() = 0;

	/// @brief Refresh block-dependent parameters at a block transition.
	virtual void updateProperties();

	/// @brief Sample from an exponential distribution with the current truck-flow rate.
	double genExponential();

	CFlowModelData_sp m_pFlowModelData;  ///< Flow model parameters (by block and lane).
	EFlowModel m_FlowModel;              ///< Active flow-model selector.
	CVehicle_sp m_pPrevVeh;              ///< Previous emitted vehicle (for minimum-gap calculation).
	CVehicle_sp m_pNextVeh;              ///< Next vehicle to be emitted.

	double m_MinGap;                     ///< Minimum physical gap in seconds (car-following constraint).
	double m_TotalFlow;                  ///< Total vehicle flow for the current block (veh/h).
	double m_TruckFlow;                  ///< Truck flow for the current block (truck/h).
	size_t m_CurBlock;                   ///< Current block index — i.e. which hour of day.
	size_t m_BlockSize;                  ///< Block size in seconds (typically 3600).
	size_t m_BlockCount;                 ///< Number of blocks in one cycle (typically 24).

	double m_MaxBridgeLength;            ///< Maximum bridge length in metres (used for multi-vehicle spacing).
	double m_BufferGapSpace;             ///< Additional safety gap in metres.
	double m_BufferGapTime;              ///< Additional safety gap in seconds.

private:
	/// @brief Advance @c m_CurBlock when @p time crosses a block boundary.
	void updateBlock(double time);

	/// @brief Refresh the exponential-distribution parameters.
	void updateExponential();

	/// @brief Compute @c m_MinGap from the previous and next vehicle velocities and lengths.
	void setMinGap();
};
typedef std::shared_ptr<CFlowGenerator> CFlowGenerator_sp;  ///< Shared-pointer alias for CFlowGenerator.


/**
 * @brief Flow generator using Headway Distribution Statistics (HeDS).
 *
 * The HeDS model samples the inter-vehicle gap from an empirical
 * cumulative distribution that is conditional on the current flow
 * rate. The per-flow-rate distributions are carried in a matrix
 * (@c m_vHeDS); speeds are drawn from a normal distribution.
 *
 * @see CFlowGenerator
 */
class CFlowGenHeDS : public CFlowGenerator
{
public:
	/**
	 * @brief Construct with the HeDS-specific model data.
	 */
	CFlowGenHeDS(CFlowModelDataHeDS_sp pFDM);
	virtual ~CFlowGenHeDS();

protected:
	double GenerateGap();          ///< Sample a gap from the current-flow-rate row of @c m_vHeDS.
	double GenerateSpeed();        ///< Sample a speed from a normal distribution.
	void updateProperties();       ///< Refresh HeDS parameters at a block transition.

private:
	CFlowModelDataHeDS_sp m_pFMD;  ///< HeDS model data.
	matrix m_vHeDS;                ///< Per-flow-rate headway cumulative distribution table.
	Normal m_Speed;                ///< Normal distribution from which speeds are drawn.
};
typedef std::shared_ptr<CFlowGenHeDS> CFlowGenHeDS_sp;  ///< Shared-pointer alias for CFlowGenHeDS.


/**
 * @brief Flow generator for congested-flow conditions.
 *
 * Models queued or congested traffic by drawing gaps from a normal
 * distribution (typically narrow around the minimum safe gap) and
 * speeds from a single constant value — representative of stop-and-go
 * or free-flowing congested conditions.
 *
 * @see CFlowGenerator
 */
class CFlowGenCongested : public CFlowGenerator
{
public:
	CFlowGenCongested(CFlowModelDataCongested_sp pFDM);
	virtual ~CFlowGenCongested();

protected:
	double GenerateGap();          ///< Sample a gap from the normal gap distribution.
	double GenerateSpeed();        ///< Return the constant congested speed.
	void updateProperties();       ///< Refresh congested-flow parameters at a block transition.

private:
	CFlowModelDataCongested_sp m_pFMD;  ///< Congested model data.
	Normal m_Gap;                       ///< Normal distribution for inter-vehicle gap.
	double m_Speed;                     ///< Constant speed in m/s.
};
typedef std::shared_ptr<CFlowGenCongested> CFlowGenCongested_sp;  ///< Shared-pointer alias for CFlowGenCongested.


/**
 * @brief Flow generator for Poisson (exponential-headway) arrivals.
 *
 * Models free-flow traffic as a Poisson process: inter-vehicle gaps
 * are drawn from an exponential distribution whose rate is set by the
 * current truck-flow rate. Speeds come from a normal distribution.
 * This is the classical assumption for light traffic.
 *
 * @see CFlowGenerator
 */
class CFlowGenPoisson : public CFlowGenerator
{
public:
	CFlowGenPoisson(CFlowModelDataPoisson_sp pFDM);
	virtual ~CFlowGenPoisson();

protected:
	double GenerateGap();          ///< Sample an exponential gap at the current flow rate.
	double GenerateSpeed();        ///< Sample a speed from the normal distribution.
	void updateProperties();       ///< Refresh Poisson parameters at a block transition.

private:
	CFlowModelDataPoisson_sp m_pFMD;  ///< Poisson model data.
	Normal m_Speed;                   ///< Normal distribution for speeds.
};
typedef std::shared_ptr<CFlowGenPoisson> CFlowGenPoisson_sp;  ///< Shared-pointer alias for CFlowGenPoisson.


/**
 * @brief Deterministic (constant-headway, constant-speed) flow generator.
 *
 * Produces perfectly regular traffic with fixed gap and speed. Used
 * mainly for testing and validation, where stochastic variation would
 * confuse the result.
 *
 * @see CFlowGenerator
 */
class CFlowGenConstant : public CFlowGenerator
{
public:
	CFlowGenConstant(CFlowModelDataConstant_sp pFDM);
	virtual ~CFlowGenConstant();

protected:
	double GenerateGap();          ///< Return the fixed configured gap.
	double GenerateSpeed();        ///< Return the fixed configured speed.

private:
	CFlowModelDataConstant_sp m_pFMD;  ///< Constant-flow model data.
};
typedef std::shared_ptr<CFlowGenConstant> CFlowGenConstant_sp;  ///< Shared-pointer alias for CFlowGenConstant.
