#include "check_ball1d_solution.h"
#include <cmath>
#include <cstdlib>

double check_ball1d_solution(double t, double h)
{
	double hh;
	int ft = (int)floor(t);
	double tau = t - ft;

	if (div(ft,2).rem == 0) // even 
	{
		hh = 1.0-tau*tau;
	}
	else
	{
		hh = tau*(2.0-tau);
	}
	return fabs(h-hh);
}

bool ball1d_soln_ok(double t, double h)
{
	return (check_ball1d_solution(t,h) < 1E-4);
}
