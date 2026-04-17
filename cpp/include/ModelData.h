/**
 * @file ModelData.h
 * @brief Interface for the CModelData class and shared traffic-model enums and helper structs.
 */

#pragma once
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <memory>
#include "CSVParse.h"
#include "MultiModalNormal.h"
#include "ConfigData.h"

/// @brief Convenience alias for a vector of doubles used throughout the model-data layer.
typedef std::vector<double> vec;

/// @brief Convenience alias for a row-major matrix of doubles.
typedef std::vector< std::vector<double> > matrix;

/**
 * @brief Selector enumerating the supported flow (headway) models.
 *
 * @see CFlowGenerator
 * @see CFlowModelData
 */
enum EFlowModel
{
	eFM_HeDS = 0,   ///< Headway Distribution Statistics (empirical).
	eFM_Poisson,    ///< Poisson arrivals (exponential gaps).
	eFM_Congested,  ///< Congested flow (narrow-normal gaps, constant speed).
	eFM_Constant    ///< Deterministic gap and speed (for testing).
};

/**
 * @brief Selector enumerating the supported vehicle models.
 *
 * @see CVehicleGenerator
 * @see CVehicleModelData
 */
enum EVehicleModel
{
	eVM_Grave = 0,  ///< Grave's parametric French-traffic model.
	eVM_Garage,     ///< Pre-recorded pool of observed vehicles.
	eVM_Constant    ///< User-supplied nominal vehicle with small perturbations.
};

/**
 * @brief Selector for the kernel shape used by randomised samplers.
 */
enum EKernelType
{
	eKT_Normal = 0,  ///< Normal (Gaussian) kernel.
	eKT_Triangle     ///< Triangular kernel.
};

/**
 * @brief Location-scale parameters for a randomisation kernel.
 */
struct KernelParams
{
	/// @brief Construct with explicit location and scale.
	KernelParams(double loc, double scale)
	{
		Location = loc;
		Scale = scale;
	};

	/// @brief Default-construct at origin (0, 0).
	KernelParams():Location(0.0), Scale(0.0){};

	double Location;  ///< Kernel centre (mean for normal, mode for triangle).
	double Scale;     ///< Kernel scale (standard deviation or half-width).
};

/**
 * @brief Simple (mean, standard deviation) pair for a univariate normal distribution.
 */
struct Normal
{
	Normal(double m, double s)
	{
		Mean = m;
		StdDev = s;
	};
	Normal():Mean(0.0), StdDev(0.0){};
	double Mean;    ///< Distribution mean.
	double StdDev;  ///< Distribution standard deviation.
};

/**
 * @brief Abstract base for model-data containers loaded from disk.
 *
 * CModelData carries the configuration reference, the filesystem path
 * to the model data folder, and a CSV parser used by derived classes
 * when loading their files. Concrete subclasses — @ref CVehicleModelData
 * and @ref CFlowModelData (and their derived specialisations) —
 * implement ReadDataIn() to populate themselves from the input files.
 *
 * @see CVehicleModelData
 * @see CFlowModelData
 */
class CModelData
{
public:
	/**
	 * @brief Construct a model-data container bound to a configuration.
	 * @param[in] config Shared configuration block.
	 */
	CModelData(CConfigDataCore& config);
	virtual ~CModelData();

	/**
	 * @brief Parse the model-specific input files.
	 *
	 * Concrete subclasses implement this to populate their internal
	 * state from the files identified in the configuration.
	 */
	virtual void ReadDataIn() = 0;

protected:
	CConfigDataCore& m_Config;         ///< Reference to the shared configuration.

	std::filesystem::path m_Path;      ///< Base filesystem path for model input files.
	CCSVParse m_CSV;                   ///< CSV parser used by ReadDataIn() implementations.
};
typedef std::shared_ptr<CModelData> CModelData_sp;  ///< Shared-pointer alias for CModelData.
