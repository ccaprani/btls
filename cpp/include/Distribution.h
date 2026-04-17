/**
 * @file Distribution.h
 * @brief Interface for CRNGWrapper and CDistribution — random number generation.
 */

#pragma once

#include "MultiModalNormal.h"
#include <random>

/**
 * @brief Process-wide random number generator wrapper.
 *
 * CRNGWrapper wraps a single static @c std::mt19937 instance shared
 * across the entire BTLS process. Every @ref CDistribution holds a
 * CRNGWrapper by value, but because the underlying generator state is
 * @c static, they all share the same stream.
 *
 * By default the static generator is seeded from @c std::random_device
 * at library-load time (non-reproducible). Calling seed() resets the
 * generator to a deterministic state, enabling reproducible runs and
 * day-level parallelism (see the Parallelisation Guide in the docs).
 *
 * In multiprocessing contexts (PyBTLS uses the @c spawn start method),
 * each worker process loads @c libbtls independently and gets its own
 * copy of the static generator. Calling @c seed(master_seed + worker_id)
 * in each worker produces independent, reproducible streams without any
 * cross-process interference.
 *
 * @see CDistribution
 */
class CRNGWrapper
{
	static std::mt19937 m_MTgen;                                   ///< Static Mersenne-Twister state shared across the process.
	static std::uniform_real_distribution<double> m_Uniform;       ///< Static uniform [0,1) distribution.
	static std::normal_distribution<double> m_Norm;                ///< Static standard-normal distribution.

public:
	/// @brief Sample a uniform [0, 1) value.
	double rand();

	/// @brief Sample a standard normal value.
	double norm();

	/**
	 * @brief Seed the process-wide Mersenne Twister generator.
	 *
	 * If never called, the generator is seeded from @c std::random_device
	 * at library load time (non-reproducible). Calling seed() makes
	 * subsequent random output deterministic for the given seed value
	 * and resets the cached distribution state so no stale values leak
	 * across reseeds.
	 *
	 * @param[in] s Seed value (unsigned 64-bit integer).
	 */
	static void seed(uint64_t s);
};

/**
 * @brief Wrapper exposing a family of random-variate generators with
 *        configurable location, scale, and shape parameters.
 *
 * CDistribution is the sampling surface used by @ref CGenerator and its
 * subclasses. It exposes generators for every distribution family
 * needed by the traffic and load-effect models: uniform, normal,
 * multi-modal normal, exponential, lognormal, gamma, Gumbel, Poisson,
 * GEV, and triangular. The active location/scale/shape parameters are
 * accumulated on the instance and fed into whichever generator is
 * invoked.
 *
 * Randomness comes from the underlying @ref CRNGWrapper, whose state
 * is static — see the note in CRNGWrapper about seeding and
 * reproducibility.
 *
 * @see CRNGWrapper
 * @see CGenerator
 */
class CDistribution
{
public:
	CDistribution();

	/**
	 * @brief Construct with explicit location, scale, and shape parameters.
	 * @param[in] loc Location parameter.
	 * @param[in] sc  Scale parameter.
	 * @param[in] sh  Shape parameter.
	 */
	CDistribution(double loc, double sc, double sh);
	virtual ~CDistribution();

	/// @brief Set the shape parameter.
	void setShape(double sh);
	/// @brief Set the scale parameter.
	void setScale(double sc);
	/// @brief Set the location parameter.
	void setLocation(double loc);

	/// @brief Get the shape parameter.
	double getShape() const;
	/// @brief Get the scale parameter.
	double getScale() const;
	/// @brief Get the location parameter.
	double getLocation() const;

	/// @brief Sample a uniform [0, 1) value.
	double GenerateUniform();

	/// @brief Sample a standard normal value using the instance parameters.
	double GenerateNormal();

	/**
	 * @brief Sample a normal value with explicit mean and standard deviation.
	 * @param[in] mean  Mean of the distribution.
	 * @param[in] stdev Standard deviation of the distribution.
	 */
	double GenerateNormal(double mean, double stdev);

	/**
	 * @brief Sample from a multi-modal normal mixture.
	 * @param[in] MMN Multi-modal normal distribution to sample from.
	 */
	double GenerateMultiModalNormal(CMultiModalNormal MMN);

	/// @brief Sample an exponential value with rate @c 1/getScale().
	double GenerateExponential();

	/// @brief Sample a lognormal value.
	double GenerateLogNormal();

	/// @brief Sample a gamma value.
	double GenerateGamma();

	/// @brief Sample a Gumbel (extreme-value type I) value.
	double GenerateGumbel();

	/// @brief Sample a Poisson value.
	double GeneratePoisson();

	/// @brief Sample from a Generalised Extreme Value distribution.
	double GenerateGEV();

	/// @brief Sample a triangular value with the instance parameters.
	double GenerateTriangular();

	/**
	 * @brief Sample a triangular value with explicit location and width.
	 * @param[in] loc Central location (mode).
	 * @param[in] w   Half-width of the triangular distribution.
	 */
	double GenerateTriangular(double loc, double w);

private:
	/// @brief Box-Muller transform producing a pair of standard normals.
	double BoxMuller();

	CRNGWrapper m_RNG;       ///< Process-wide RNG wrapper.

	const double PI;         ///< Cached pi used by the Box-Muller transform.

	double m_Location;       ///< Active location parameter.
	double m_Scale;          ///< Active scale parameter.
	double m_Shape;          ///< Active shape parameter.
};
