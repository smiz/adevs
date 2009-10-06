#ifndef _adevs_corrected_euler_h_
#define _adevs_corrected_euler_h_
#include <cmath>
#include "adevs_hybrid.h"

namespace adevs
{

/**
 * This is the second order accurate RK2 method with adaptive step sizing for
 * error control.
 */
template <typename X> class corrected_euler:
	public ode_solver<X>
{
	public:
		/**
		 * Create an integrator that will use the specified per step error
		 * tolerance and maximum step size.
		 */
		corrected_euler(ode_system<X>* sys, double err_tol, double h_max);
		/// Destructor
		~corrected_euler();
		double integrate(double* q, double h_lim);
		void advance(double* q, double h);
	private:
		double *dq, // derivative
			   *qq, // trial solution
			   *t,  // temporary variable for computing k2
			   *k[2]; // k1 and k2
		const double err_tol; // Error tolerance
		const double h_max; // Maximum time step
		double h_cur; // Previous time step that satisfied error constraint
		// Compute a step of size h, put it in qq, and return the error
		double trial_step(double h);
};

template <typename X>
corrected_euler<X>::corrected_euler(ode_system<X>* sys, double err_tol,
		double h_max):
	ode_solver<X>(sys),err_tol(err_tol),h_max(h_max),h_cur(h_max)
{
	for (int i = 0; i < 2; i++) k[i] = new double[sys->numVars()];
	dq = new double[sys->numVars()];
	qq = new double[sys->numVars()];
	t = new double[sys->numVars()];
}

template <typename X>
corrected_euler<X>::~corrected_euler()
{
	delete [] t; delete [] qq; delete [] dq;
	for (int i = 0; i < 2; i++) delete [] k[i];
}

template <typename X>
void corrected_euler<X>::advance(double* q, double h)
{
	double dt;
	while ((dt = integrate(q,h)) < h) h -= dt;
}

template <typename X>
double corrected_euler<X>::integrate(double* q, double h_lim)
{
	// Initial error estimate and step size
	double err = DBL_MAX, h = std::min(h_cur*1.1,std::min(h_max,h_lim));
	for (;;) {
		// Copy q to the trial vector
		for (int i = 0; i < this->sys->numVars(); i++) qq[i] = q[i];
		// Make the trial step which will be stored in qq
		err = trial_step(h);
		// If the error is ok, then we have found the proper step size
		if (err <= err_tol) { // Keep h if shrunk to control the error
			if (h_lim >= h_cur) h_cur = h; 
			break;
		}
		// Otherwise shrink the step size and try again
		else {
			double h_guess = 0.8*err_tol*h/fabs(err);
			if (h < h_guess) h *= 0.8;
			else h = h_guess;
		}
	}
	// Put the trial solution in q and return the selected step size
	for (int i = 0; i < this->sys->numVars(); i++) q[i] = qq[i];
	return h;
}

template <typename X>
double corrected_euler<X>::trial_step(double step)
{
	int j;
	// Compute k1
	this->sys->der_func(qq,dq); 
	for (j = 0; j < this->sys->numVars(); j++) k[0][j] = step*dq[j];
	// Compute k2
	for (j = 0; j < this->sys->numVars(); j++) t[j] = qq[j] + 0.5*k[0][j];
	this->sys->der_func(t,dq);
	for (j = 0; j < this->sys->numVars(); j++) k[1][j] = step*dq[j];
	// Compute next state and approximate error
	double err = 0.0;
	for (j = 0; j < this->sys->numVars(); j++) {
		qq[j] += k[1][j]; // Next state
		err = std::max(err,fabs(k[0][j]-k[1][j])); // Maximum error
	}
	return err; // Return the error
}

} // end of namespace
#endif
