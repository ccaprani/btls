/**
 * @file InfluenceLine.h
 * @brief Interface for the CInfluenceLine class — load-effect influence lines and surfaces.
 */

#pragma once

#include <vector>
#include "Axle.h"
#include "InfluenceSurface.h"

/**
 * @brief Influence line, discrete ordinate table, or influence surface used
 *        to compute a single load effect from a set of axles.
 *
 * CInfluenceLine supports three underlying representations, selected at
 * construction time:
 *
 * 1. **Analytical expression** (type 1): one of nine hard-coded closed-form
 *    influence lines for common simply-supported and multi-span
 *    bending/shear cases. See the individual @c LoadEffect* functions below.
 * 2. **Discrete ordinates** (type 2): a user-supplied table of (distance,
 *    ordinate) pairs; the ordinate at any position is obtained by linear
 *    interpolation.
 * 3. **Influence surface** (type 3): a two-dimensional influence surface
 *    delegating to @ref CInfluenceSurface. For type-3 influence lines,
 *    each axle's load is split equally between the two wheel paths at
 *    ±TrackWidth/2 from the axle centreline.
 *
 * A per-effect scaling weight (e.g. a girder distribution factor) is
 * applied to the type-1 and type-2 ordinates via setWeight(); it is not
 * applied to a type-3 influence surface. For type-1 expressions, a
 * function pointer is cached in setIL() so the inner loop does not need
 * to branch on the function index for each axle.
 *
 * Available type-1 functions (see LoadEffect1–LoadEffect9):
 *
 *   1. Mid-span bending moment, simply-supported beam
 *   2. Bending moment over central support, two-span beam
 *   3. Left-hand shear, simply-supported beam
 *   4. Right-hand shear, simply-supported beam
 *   5. Left-hand shear, two-span beam
 *   6. Right-hand shear, two-span beam
 *   7. Total load on the beam (constant = 1)
 *   8. Bending moment over second support, three-span beam
 *   9. Bending moment over third support, three-span beam
 *
 * @see CInfluenceSurface
 * @see CAxle
 * @see CBridgeLane
 */
class CInfluenceLine
{
public:
	CInfluenceLine(void);
	~CInfluenceLine(void);

	/**
	 * @brief Get the influence line ordinate at longitudinal position @p x.
	 *
	 * For type-1 and type-2 influence lines, returns the ordinate times
	 * the scaling weight. For type-3 surfaces, callers should use
	 * getLoadEffect() or @ref CInfluenceSurface directly — this method
	 * does not handle the transverse coordinate.
	 *
	 * @param[in] x Longitudinal position along the influence line, in metres.
	 * @return Ordinate times the configured weight.
	 */
	double getOrdinate(double x);

	/// @brief Get the number of discrete points (type 2 only).
	size_t getNoPoints(void);

	/**
	 * @brief Compute the total load effect for a set of axles.
	 *
	 * Sums the per-axle contribution for every axle in @p vAxle. For
	 * type-3 surfaces, each axle's load is split across the two wheel
	 * paths at ±TrackWidth/2; for types 1 and 2 the full axle weight is
	 * applied at the axle's longitudinal position.
	 *
	 * @param[in,out] vAxle Axles contributing to the load effect.
	 * @return Total load effect in the influence line's native units
	 *         (e.g. kN·m for a bending moment).
	 */
	double getLoadEffect(std::vector<CAxle>& vAxle);

	/// @brief Get the type tag: 1 = analytical, 2 = discrete, 3 = surface.
	size_t getType() { return m_Type; };

	/// @brief Get a pointer to the influence surface (type 3 only).
	CInfluenceSurface* getIS();

	/**
	 * @brief Configure the influence line as one of the built-in analytical
	 *        expressions.
	 *
	 * Sets the type tag to 1 and caches a function pointer to the
	 * corresponding @c LoadEffect function so the inner loop does not
	 * branch on the function index.
	 *
	 * @param[in] nIL    Function index in 1..9; see the class-level list.
	 * @param[in] length Span length in metres.
	 */
	void setIL(size_t nIL, double length);

	/**
	 * @brief Configure the influence line as a discrete ordinate table.
	 *
	 * Sets the type tag to 2. Values between tabulated points are linearly
	 * interpolated; values outside the tabulated range return zero.
	 *
	 * @param[in] vDis Distances along the beam in metres; must be strictly increasing.
	 * @param[in] vOrd Ordinates at each distance, in the load effect's native units.
	 */
	void setIL(std::vector<double> vDis, std::vector<double> vOrd);

	/**
	 * @brief Configure the influence line as a two-dimensional surface.
	 *
	 * Sets the type tag to 3. Ordinate lookups delegate to
	 * @ref CInfluenceSurface.
	 *
	 * @param[in] IS Influence surface.
	 */
	void setIL(CInfluenceSurface IS);

	/// @brief Get the length of the influence line in metres.
	double getLength(void);

	/// @brief Set a reference index for this influence line.
	void setIndex(size_t n);

	/// @brief Get the reference index for this influence line.
	size_t getIndex(void);

	/**
	 * @brief Set the scaling weight applied to the type-1 and type-2 ordinates.
	 *
	 * Typically used to apply a girder distribution factor or a
	 * unit-conversion constant. The weight has no effect on a type-3
	 * influence surface: surface load effects are not scaled by it.
	 *
	 * @param[in] weight Scaling factor; default is 1.0.
	 */
	void setWeight(double weight);

	/**
	 * @brief Load-effect mode tag for the per-axle force formula.
	 *
	 * The bridge-geometry factors (radius R, superelevation factor k_e for
	 * centrifugal; lever-arm or design constants for braking) are intentionally
	 * kept on the caller side: the caller bakes them into the influence-line
	 * ordinates (or, for type-1 and type-2 influence lines only, applies them
	 * via @ref setWeight) so that the IL remains the single source of
	 * bridge-specific physics. The C++ dispatch below only selects the
	 * per-axle force proxy:
	 *
	 * - Vertical (default): F_axle = AxleWeight; ordinary vertical reaction.
	 * - Centrifugal: F_axle = AxleWeight * Speed^2 / g, taking the per-axle
	 *   speed m_Speed (per-vehicle, set by the upstream traffic generator).
	 *   The caller is expected to supply an IL whose ordinates carry the
	 *   k_e / R factor so that the convolution yields a force in the same
	 *   units as the vertical reaction.
	 * - Braking: F_axle = AxleWeight * |Acceleration| / g, taking the per-axle
	 *   m_Acceleration (per-vehicle / per-time, set by the upstream solver).
	 *   If m_Acceleration is exactly zero, falls back to AxleWeight times the
	 *   scalar @ref setBrakingFactor (= a_design / g) for code-prescribed
	 *   constant-deceleration cases.
	 */
	enum LoadEffectMode {
		LE_Vertical    = 0,
		LE_Centrifugal = 1,
		LE_Braking     = 2
	};

	/**
	 * @brief Select the load-effect mode used in @ref getLoadEffect.
	 *
	 * @param[in] mode One of LE_Vertical (0), LE_Centrifugal (1), LE_Braking (2).
	 * @throws std::invalid_argument if @p mode is outside that range.
	 */
	void setLoadEffectMode(size_t mode);

	/**
	 * @brief Configure the dimensionless braking-mode scalar fallback.
	 *
	 * Used in braking mode whenever the per-axle CAxle::m_Acceleration is zero
	 * (for example, when the upstream traffic generator does not write a
	 * per-vehicle deceleration). The per-axle braking force becomes
	 *   F_B = AxleWeight * brakingFactor
	 * with @p brakingFactor interpreted as a dimensionless deceleration ratio
	 * (a / g) for a code-prescribed constant-deceleration design rule.
	 *
	 * @param[in] brakingFactor Dimensionless braking ratio (deceleration / g).
	 */
	void setBrakingFactor(double brakingFactor);

private:
	/// @brief Compute the load effect contribution of a single axle.
	double getAxleLoadEffect(CAxle& axle);

	/// @brief Dispatch to the cached analytical ordinate function (type 1).
	double getEquationOrdinate(double x);

	/// @brief Interpolate the ordinate from the discrete table (type 2).
	double getDiscreteOrdinate(double x);

	/// @brief Mid-span bending moment, simply-supported beam.
	double LoadEffect1(double x);
	/// @brief Bending moment over central support, two-span beam.
	double LoadEffect2(double x);
	/// @brief Left-hand shear, simply-supported beam.
	double LoadEffect3(double x);
	/// @brief Right-hand shear, simply-supported beam.
	double LoadEffect4(double x);
	/// @brief Left-hand shear, two-span beam.
	double LoadEffect5(double x);
	/// @brief Right-hand shear, two-span beam.
	double LoadEffect6(double x);
	/// @brief Total load on the beam (constant = 1).
	double LoadEffect7(double x);
	/// @brief Bending moment over second support, three-span beam.
	double LoadEffect8(double x);
	/// @brief Bending moment over third support, three-span beam.
	double LoadEffect9(double x);

	size_t m_Type;                    ///< 1 = expression, 2 = discrete, 3 = surface.
	size_t m_ILFunctionNo;            ///< Analytical function index (type 1 only).
	size_t m_Index;                   ///< Reference index set by setIndex().
	double m_Length;                  ///< Span length in metres.
	size_t m_NoPoints;                ///< Number of discrete ordinate points (type 2).
	double m_Weight;                  ///< Scaling weight applied to all ordinates.
	std::vector<double> m_vDistance;  ///< Discrete distances in metres (type 2).
	std::vector<double> m_vOrdinate;  ///< Discrete ordinates (type 2).

	CInfluenceSurface m_IS;           ///< Influence surface (type 3).

	/// @brief Pointer-to-member type for analytical load-effect functions.
	typedef double (CInfluenceLine::*LEfptr)(double x);

	std::vector<LEfptr> m_vLEfptr;    ///< Dispatch table for type-1 load-effect functions.
	LEfptr m_LEfptr;                  ///< Cached function pointer for the current analytical expression.

	size_t m_LoadEffectMode;          ///< Per-axle force formula: 0 = vertical, 1 = centrifugal, 2 = braking.
	double m_BrakingFactor;           ///< Dimensionless braking ratio fallback (braking mode only); used when CAxle::m_Acceleration is zero.
};
