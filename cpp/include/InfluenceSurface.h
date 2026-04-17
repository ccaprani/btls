/**
 * @file InfluenceSurface.h
 * @brief Interface for the CInfluenceSurface class — two-dimensional influence surface.
 */

#pragma once

#include <vector>
#include <math.h>

/**
 * @brief A two-dimensional influence surface used to compute load effects
 *        that depend on both longitudinal position and transverse wheel
 *        placement.
 *
 * CInfluenceSurface stores a regular grid of ordinates indexed by
 * longitudinal (x) and transverse (y) coordinates. The ordinate at an
 * arbitrary (x, y) is obtained by bilinear interpolation over the grid.
 *
 * Used when CInfluenceLine is in type-3 (surface) mode: each axle's
 * load effect is computed by looking up the ordinate at both wheel
 * paths (±TrackWidth/2 from the axle centreline) and averaging.
 *
 * @see CInfluenceLine
 */
class CInfluenceSurface
{
public:
	CInfluenceSurface(void);

	/**
	 * @brief Construct with a grid of ordinates and per-lane y-coordinates.
	 *
	 * @param[in] ISmat  Row-major ordinate grid (rows = x stations, cols = y stations).
	 * @param[in] ylanes Y-coordinates of lane centre-lines.
	 */
	CInfluenceSurface(std::vector< std::vector<double> > ISmat, std::vector<double> ylanes);
	virtual ~CInfluenceSurface(void);

	/**
	 * @brief Get the ordinate at position (x, y) for a given lane.
	 *
	 * @param[in] x               Longitudinal position in metres.
	 * @param[in] laneEccentricity Transverse offset from the lane centreline in metres.
	 * @param[in] iLane            Zero-based lane index.
	 * @return Interpolated ordinate.
	 */
	double giveOrdinate(double x, double laneEccentricity, std::size_t iLane);

	/// @brief Set the ordinate grid from a row-major matrix.
	void setIS(std::vector< std::vector<double> > ISmat);

	/// @brief Set lane y-coordinates from a flat vector.
	void setLanes(std::vector<double> ylanes);

	/// @brief Set lane y-coordinates from (ymin, ymax) pairs.
	void setLanes(std::vector<std::pair<double,double>> ylanes);

	/// @brief Get the longitudinal length of the surface in metres.
	double getLength();

	/**
	 * @brief Get the lane width for lane @p iLane, in metres.
	 * @param[in] iLane Zero-based lane index.
	 */
	double getLaneWidth(std::size_t iLane);

private:
	std::vector< std::vector<double> > m_ISords;  ///< Row-major ordinate grid.
	std::vector<double> m_X;                      ///< Longitudinal station coordinates in metres.
	std::vector<double> m_Y;                      ///< Transverse station coordinates in metres.
	std::vector<std::pair<double,double>> m_Ylanes;///< Per-lane (ymin, ymax) pairs.

	double m_Xmin, m_Xmax;   ///< Longitudinal extents.
	double m_Ymin, m_Ymax;   ///< Transverse extents.
	double m_Length;          ///< Surface length in metres.
	std::size_t m_NoX;       ///< Number of longitudinal stations.
	std::size_t m_NoY;       ///< Number of transverse stations.
	std::size_t m_NoLanes;   ///< Number of lanes on the surface.
};
