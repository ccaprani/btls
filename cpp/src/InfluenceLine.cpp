#include "InfluenceLine.h"
#include <algorithm>

#include <cmath>
#include <stdexcept>


// Acceleration due to gravity used in centrifugal- and braking-mode
// force conversion. CAxle::m_AxleWeight is in kN (= mass[kg] * g[m/s^2]
// / 1000); dividing by g recovers an axle "mass coefficient" so that
// the per-axle centrifugal force AxleWeight * v^2 / g (kN.m, before
// IL convolution that bakes in 1/R) and braking force
// AxleWeight * |a| / g (kN) come out in the right units.
static constexpr double GRAVITY_MS2_FOR_LE = 9.80665;

CInfluenceLine::CInfluenceLine(void)
	: m_Type(0), m_Weight(1.0)
	, m_LoadEffectMode(0)
	, m_BrakingFactor(0.0)
{
	// Type: 1 - expression, 2 - discrete, 3 - Surface
	// LoadEffectMode: 0 - vertical (default), 1 - centrifugal, 2 - braking

	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect1);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect2);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect3);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect4);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect5);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect6);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect7);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect8);
	m_vLEfptr.push_back(&CInfluenceLine::LoadEffect9);
}


CInfluenceLine::~CInfluenceLine(void)
{
}

CInfluenceSurface* CInfluenceLine::getIS()
{
	return &m_IS;
}

// get the load effect value given an axle vector
double CInfluenceLine::getLoadEffect(std::vector<CAxle>& vAxles)
{
	double effVal = 0.0;
	for(unsigned int j = 0; j < vAxles.size(); j++)
		effVal += getAxleLoadEffect(vAxles[j]);
	return effVal;
}

double CInfluenceLine::getAxleLoadEffect(CAxle& axle)
{
	double effVal = 0.0;

	// Per-axle force coefficient: depends on the load-effect mode.
	// Bridge-geometry constants (radius R, superelevation factor k_e for
	// centrifugal; lever-arm or design constants for braking) are baked
	// into the influence-line ordinates by the caller at IL construction
	// time. For type-1 and type-2 influence lines they may instead be
	// applied through setWeight(); the weight is deliberately NOT applied
	// to a type-3 influence surface (see the type-3 branch below), so it
	// cannot carry them there. The C++ side computes only the per-axle
	// kinematic force proxy.
	//   Vertical    (0): F_axle = AxleWeight                              (default).
	//   Centrifugal (1): F_axle = AxleWeight * Speed^2 / g                (per-vehicle v^2).
	//                    Caller bakes k_e / R into the IL ordinates so that
	//                    the convolved bearing reaction is in kN.
	//   Braking     (2): F_axle = AxleWeight * |Acceleration| / g         (per-vehicle deceleration);
	//                    falls back to AxleWeight * |brakingFactor| if Acceleration is zero, where
	//                    brakingFactor is dimensionless (deceleration / g) configured via
	//                    @ref setBrakingFactor for code-prescribed constant-deceleration cases.
	double force_coeff = axle.m_AxleWeight;
	if (m_LoadEffectMode == LE_Centrifugal)
	{
		double v = axle.m_Speed;
		force_coeff = axle.m_AxleWeight * (v * v) / GRAVITY_MS2_FOR_LE;
	}
	else if (m_LoadEffectMode == LE_Braking)
	{
		// Braking force on each axle is mass * |deceleration|.
		// Mass = AxleWeight / g; |a| from the per-axle m_Acceleration (set by the upstream
		// traffic generator, e.g. an IDM-active solver writing per-time deceleration into
		// each axle). If m_Acceleration is exactly zero (e.g. a constant-velocity stream
		// without an IDM-driven deceleration), the scalar m_BrakingFactor (= a_design / g)
		// configured via setBrakingFactor() is used as a code-prescribed-design fallback.
		double a_over_g = (axle.m_Acceleration != 0.0)
			? std::abs(axle.m_Acceleration) / GRAVITY_MS2_FOR_LE
			: m_BrakingFactor;
		force_coeff = axle.m_AxleWeight * a_over_g;
	}

	if(m_Type == 3)	// Influence surface
	{
		double ord1 = m_IS.giveOrdinate(axle.m_Position,axle.m_Eccentricity-axle.m_TrackWidth/2,axle.m_Lane);
		double ord2 = m_IS.giveOrdinate(axle.m_Position,axle.m_Eccentricity+axle.m_TrackWidth/2,axle.m_Lane);
		// m_Weight is intentionally not applied here: an influence surface is
		// not scaled by the per-lane influence weight.
		effVal = 0.5*force_coeff*(ord1+ord2); // assumes half axle force on each wheel
	}
	else
		effVal = force_coeff*getOrdinate(axle.m_Position);

	return effVal;
}


// get the ordinate at the position x
double CInfluenceLine::getOrdinate(double x)
{
	double ord = 0.0;
	if(m_Type == 2)
		ord = getDiscreteOrdinate(x);
	else
		ord = getEquationOrdinate(x);
	return ord * m_Weight;
}


// set the influence line no.
void CInfluenceLine::setIL(size_t nIL, double length)
{
	m_Type = 1;
	m_ILFunctionNo = nIL;
	m_Length = length;
	// set the function pointer to the correct LE equation
	m_LEfptr = m_vLEfptr.at(m_ILFunctionNo-1); // -1 zero based array
}

// set the influence line data
void CInfluenceLine::setIL(std::vector<double> vDis, std::vector<double> vOrd)
{
	m_Type = 2;
	m_vDistance = vDis;
	m_vOrdinate = vOrd;
	m_NoPoints = m_vDistance.size();
	m_Length = m_vDistance.at(m_NoPoints-1);
}

// Set the influence surface
void CInfluenceLine::setIL(CInfluenceSurface IS)
{
	m_Type = 3;
	m_IS = IS;
	m_Length = m_IS.getLength();
}

double CInfluenceLine::getLength(void)
{
	return m_Length;
}


void CInfluenceLine::setIndex(size_t n)
{
	m_Index = n;
}

size_t CInfluenceLine::getIndex(void)
{
	return m_Index;
}

void CInfluenceLine::setWeight(double weight)
{
	m_Weight = weight;
}

void CInfluenceLine::setLoadEffectMode(size_t mode)
{
	// 0 = vertical (default), 1 = centrifugal, 2 = braking.
	if(mode > LE_Braking)
		throw std::invalid_argument("Load effect mode must be 0 (vertical), 1 (centrifugal) or 2 (braking).");
	m_LoadEffectMode = mode;
}

void CInfluenceLine::setBrakingFactor(double brakingFactor)
{
	m_BrakingFactor = brakingFactor;
}

double CInfluenceLine::getDiscreteOrdinate(double x)
{
	// right at the end of the IL
	if(x >= m_Length - 0.001 && x <= m_Length + 0.001)
		return m_vOrdinate[m_NoPoints-1];
	// not on the IL
	else if(x < m_vDistance[0] || x > m_Length)
		return 0.0;
	// On the IL, but not at the end
	else
	{
		// first index with m_vDistance[i] > x - same as the former linear scan
		size_t i = std::upper_bound(m_vDistance.begin(), m_vDistance.end(), x) - m_vDistance.begin();
		double deltaX = m_vDistance[i] - m_vDistance[i-1];
		double ord1 = m_vOrdinate[i-1];
		double ord2 = m_vOrdinate[i];
		double ordinate = ord1 + (x-m_vDistance[i-1])/deltaX*(ord2-ord1);
		return ordinate;
	}
}

double CInfluenceLine::getEquationOrdinate(double x)
{
	// using function pointers to select correct LE equation
	return (this->*m_LEfptr)(x);
}

size_t CInfluenceLine::getNoPoints(void)
{
	return m_NoPoints;
}

// Mid-span bending moment: simply-supported beam
double CInfluenceLine::LoadEffect1(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;
	
	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = x * (1 - s/L);
		else
			ordinate = s * (1 - x/L);
	}
	return ordinate;
}

// bending moment over central support of two-span beam
double CInfluenceLine::LoadEffect2(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = x * (s*s - x*x) / (4*s*s);   
		else
		{
			double y = 2 * s - x;
			ordinate = y * (s*s - y*y) / (4*s*s);
		}
	}
	return ordinate;
}

// left hand shear in simply supported beam
double CInfluenceLine::LoadEffect3(double x)
{
	double L = m_Length;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		ordinate = 1 - x / L;
	}
	return ordinate;
}

// right hand shear in simply-supported beam
double CInfluenceLine::LoadEffect4(double x)
{
	double L = m_Length;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		ordinate = x / L;
	}
	return ordinate;
}

// left hand shear for two-span beam
double CInfluenceLine::LoadEffect5(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;

	if((x <= 0.0) || (x >= L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = (1/s) * (-x * (s*s - x*x) / (4*s*s) + s - x);
		else
		{
			double y = 2 * s - x;
			ordinate = (1/s) * (-y * (s*s - y*y) / (4*s*s) );
		}
	}
	return ordinate;
}

// right hand shear for two-span beam
double CInfluenceLine::LoadEffect6(double x)
{
	double L = m_Length;
	double s = L/2;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
			ordinate = (1/s) * (-x * (s*s - x*x) / (4*s*s) );
		else
		{
			double y = 2 * s - x;
			ordinate = (1/s) * (-y * (s*s - y*y) / (4*s*s) + s - y);
		}
	}
	return ordinate;
}

// total load on the beam
double CInfluenceLine::LoadEffect7(double x)
{
	if((x < 0.0) || (x > m_Length))
		return 0.0;
	else
		return 1.0;
}

// bending moment over the second support of three-span beam
double CInfluenceLine::LoadEffect8(double x)
{
	double L = m_Length;
	double s = L/3;
	double K = 15*s*s;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
		{
			ordinate = 2 * x/s * (1 - x/s) * (1 + x/s) * s*s * (2*s) / K;
		}
		else if(x < 2*s)
		{
			x = x - s;
			ordinate = x/s * (1 - x/s) * s*s * (3 * s * (1 - x/s) + 2 * s * (2 - x/s)) / K;
		}
		else
		{
			x = x - 2*s;
			ordinate = -x/s * (1 - x/s) * (2 - x/s) * s*s*s / K;
		}
	}
	return ordinate;
}

// bending moment over the third support of three-span beam
double CInfluenceLine::LoadEffect9(double x)
{
	double L = m_Length;
	double s = L/3;
	double K = 15*s*s;
	double ordinate = 0.0;

	if((x < 0.0) || (x > L))
		ordinate = 0.0;
	else
	{
		if(x < s)
		{
			ordinate = x/s * (1 - x/s) * (1 + x/s) * s*s*s / -K;
		}
		else if(x < 2*s)
		{
			x = x - s;
			ordinate = x/s * (1 - x/s) * s*s * (3 * x/s * s + 2 * s * (1 + x/s)) / K;
		}
		else
		{
			x = x - 2*s;
			ordinate = 2 * x/s * (1 - x/s) * (2 - x/s) * s*s * (s + s) / K;
		}
	}
	return ordinate;
}
