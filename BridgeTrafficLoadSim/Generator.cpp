#include "Generator.h"


CGenerator::CGenerator()
{
}


CGenerator::~CGenerator()
{
}

double CGenerator::quad(double a, double b, double c, const double x)
{
	return a*x*x + b*x + c;
}

double CGenerator::inv_quad(double a, double b, double c, const double y)
{
	c = c - y;
	double x1 = (-b + sqrt(b*b - 4 * a*c)) / (2 * a);
	double x2 = (-b - sqrt(b*b - 4 * a*c)) / (2 * a);
	double x0 = -b / (2 * a); // this is the derivative at the min point

	if (a < 0 && x1 > x0) // ie concave
		return x1;
	else
	{
		c = c + y;
		double y_trial = quad(a, b, c, x1);
		if (y_trial > 0.99*y && y_trial < 1.01*y)
			return x1;
		else
			return x2;
	}
}
