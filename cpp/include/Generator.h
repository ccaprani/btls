/**
 * @file Generator.h
 * @brief Interface for the CGenerator class — abstract base for vehicle and flow generators.
 */

#pragma once
#include <memory>
#include "ModelData.h"
#include "Distribution.h"

/**
 * @brief Abstract base class for stochastic traffic generators.
 *
 * CGenerator provides the shared state — a model-data pointer and a
 * random-number generator wrapper — used by both @ref CVehicleGenerator
 * (which produces vehicle geometry and weights) and
 * @ref CFlowGenerator (which produces arrival gaps and speeds). It also
 * exposes a handful of protected numerical helpers used by the derived
 * sampling routines.
 *
 * @see CVehicleGenerator
 * @see CFlowGenerator
 * @see CModelData
 * @see CDistribution
 */
class CGenerator
{
public:
	CGenerator();
	virtual ~CGenerator();

protected:
	/**
	 * @brief Invert a quadratic @c a*x^2 + b*x + c for x given y.
	 * @param[in] a,b,c Coefficients.
	 * @param[in] y     Target ordinate.
	 * @return Value of x solving the quadratic.
	 */
	double	inv_quad(double a, double b, double c, double y);

	/**
	 * @brief Evaluate a quadratic @c a*x^2 + b*x + c at x.
	 * @param[in] a,b,c Coefficients.
	 * @param[in] x     Input value.
	 */
	double	quad(double a, double b, double c, double x);

	/**
	 * @brief Scale every element of @p vec in place by @p scale.
	 */
	void	ScaleVector(std::vector<double>& vec, double scale);

	/**
	 * @brief Sum the elements of @p vec.
	 */
	double	SumVector(std::vector<double> vec);

	CModelData* m_pModelData;  ///< Non-owning pointer to model data (weights, spacings, flow rates, …).
	CDistribution m_RNG;       ///< Random number generator wrapper for sampling distributions.
};
typedef std::shared_ptr<CGenerator> CGenerator_sp;  ///< Shared-pointer alias for CGenerator.
