/**
 * @file CalcEffect.h
 * @brief Interface for the CCalcEffect class — legacy analytical load-effect calculator.
 */

#if !defined(AFX_CALCEFFECT_H__3FD6FAB1_0A36_43E9_9240_1CBF454DABF4__INCLUDED_)
#define AFX_CALCEFFECT_H__3FD6FAB1_0A36_43E9_9240_1CBF454DABF4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
 * @brief Unused analytical load-effect calculator.
 *
 * Provides closed-form load-effect functions (effect1–7, effect16, A1/A2,
 * B1/B2, C1/C2, Beam1–5) for a point load on a beam of a given span. The
 * class is instantiated as a member of @ref CBridge but its methods are
 * not invoked by the current simulation path in either the standalone
 * BTLS binary or the PyBTLS extension; all load effects are routed
 * through @ref CInfluenceLine instead.
 *
 * @note New code should use @ref CInfluenceLine instead. The influence-line
 *       framework supports arbitrary discrete ordinates and influence
 *       surfaces, neither of which this class handles.
 *
 * @see CInfluenceLine
 */
class CCalcEffect
{
public:
	/**
	 * @brief Set the span of the beam.
	 * @param[in] span Beam span in metres.
	 */
	void setSpan(double span);

	CCalcEffect();

	/**
	 * @brief Construct with a beam span.
	 * @param[in] span Beam span in metres.
	 */
	CCalcEffect(double span);
	virtual ~CCalcEffect();

	/**
	 * @brief Evaluate one of the analytical effect functions for a point load.
	 *
	 * @param[in] j    Effect-function index (selects give_effect1–16, A/B/C, Beam1–5).
	 * @param[in] X    Longitudinal position of the load along the beam, in metres.
	 * @param[in] AW   Axle weight in kN.
	 * @param[in] lane Lane index, passed through to functions that are lane-dependent.
	 * @return Load effect value in the function's native units.
	 */
	double giveEffect(int j, double X, double AW, int lane);

private:
	double m_span;                      ///< Beam span in metres.
	double give_effect1(double x);      ///< Legacy analytical effect function.
	double give_effect2(double x);      ///< Legacy analytical effect function.
	double give_effect3(double x);      ///< Legacy analytical effect function.
	double give_effect4(double x);      ///< Legacy analytical effect function.
	double give_effect5(double x);      ///< Legacy analytical effect function.
	double give_effect6(double x);      ///< Legacy analytical effect function.
	double give_effect7(double x);      ///< Legacy analytical effect function.
	double give_effectA1(double x);     ///< Legacy analytical effect function.
	double give_effectA2(double x);     ///< Legacy analytical effect function.
	double give_effectB1(double x);     ///< Legacy analytical effect function.
	double give_effectB2(double x);     ///< Legacy analytical effect function.
	double give_effectC1(double x);     ///< Legacy analytical effect function.
	double give_effectC2(double x);     ///< Legacy analytical effect function.
	double give_Beam1(double x);        ///< Legacy analytical beam function.
	double give_Beam2(double x);        ///< Legacy analytical beam function.
	double give_Beam3(double x);        ///< Legacy analytical beam function.
	double give_Beam4(double x);        ///< Legacy analytical beam function.
	double give_Beam5(double x);        ///< Legacy analytical beam function.
	double give_effect16(double x);     ///< Legacy analytical effect function.
};

#endif // !defined(AFX_CALCEFFECT_H__3FD6FAB1_0A36_43E9_9240_1CBF454DABF4__INCLUDED_)
