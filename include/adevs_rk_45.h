/**
 * Copyright (c) 2013, James Nutaro
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 * Bugs, comments, and questions can be sent to nutaro@gmail.com
 */
#ifndef _adevs_rk_45_h_
#define _adevs_rk_45_h_
#include "adevs_hybrid.h"
#include <cmath>

namespace adevs
{

/**
 * This ode_solver implements a 4th/5th order integrator that adjust
 * its step size to control error.
 */
template <typename X> class rk_45:
	public ode_solver<X>
{
	public:
		/**
		 * The integrator will adjust its step size to maintain a per
		 * step error less than err_tol, and will use a step size
		 * strictly less than h_max.
		 */
		rk_45(ode_system<X>* sys, double err_tol, double h_max);
		/// Destructor
		~rk_45();
		double integrate(double* q, double h_lim);
		void advance(double* q, double h);
	private:
		double *dq, // derivative
			   *qq, // trial solution
			   *t,  // temporary variables for computing stages
			   *k[6]; // the six RK stages
		const double err_tol; // Error tolerance
		const double h_max; // Maximum time step
		double h_cur; // Previous successful step size
		// Compute a trial step of size h, store the result in qq, and return the error
		double trial_step(double h);
};

template <typename X>
rk_45<X>::rk_45(ode_system<X>* sys, double err_tol, double h_max):
	ode_solver<X>(sys),err_tol(err_tol),h_max(h_max),h_cur(h_max)
{
	for (int i = 0; i < 6; i++)
		k[i] = new double[sys->numVars()];
	dq = new double[sys->numVars()];
	qq = new double[sys->numVars()];
	t = new double[sys->numVars()];
}

template <typename X>
rk_45<X>::~rk_45()
{
	delete [] dq;
	delete [] t;
	for (int i = 0; i < 6; i++)
		delete [] k[i];
}

template <typename X>
void rk_45<X>::advance(double* q, double h)
{
	double dt;
	while ((dt = integrate(q,h)) < h) h -= dt;
}

template <typename X>
double rk_45<X>::integrate(double* q, double h_lim)
{
	// Initial error estimate and step size
	double err = DBL_MAX, h = std::min(h_cur*1.1,std::min(h_max,h_lim));
	for (;;) {
		// Copy q to the trial vector
		for (int i = 0; i < this->sys->numVars(); i++) qq[i] = q[i];
		// Make the trial step which will be stored in qq
		err = trial_step(h);
		// If the error is ok, then we have found the proper step size
		if (err <= err_tol) {
			if (h_cur <= h_lim) h_cur = h;
			break;
		}
		// Otherwise shrink the step size and try again
		else {
			double h_guess = 0.8*pow(err_tol*pow(h,4.0)/fabs(err),0.25);
			if (h < h_guess) h *= 0.8;
			else h = h_guess;
		}
	}
	// Copy the trial solution to q and return the step size that was selected
	for (int i = 0; i < this->sys->numVars(); i++) q[i] = qq[i];
	return h;
}

template <typename X>
double rk_45<X>::trial_step(double step)
{
	// Compute k1
	this->sys->der_func(qq,dq); 
	for (int j = 0; j < this->sys->numVars(); j++) k[0][j] = step*dq[j];
	// Compute k2
	for (int j = 0; j < this->sys->numVars(); j++) t[j] = qq[j] + 0.5*k[0][j];
	this->sys->der_func(t,dq);
	for (int j = 0; j < this->sys->numVars(); j++) k[1][j] = step*dq[j];
	// Compute k3
	for (int j = 0; j < this->sys->numVars(); j++) t[j] = qq[j] + 0.25*(k[0][j]+k[1][j]);
	this->sys->der_func(t,dq);
	for (int j = 0; j < this->sys->numVars(); j++) k[2][j] = step*dq[j];
	// Compute k4
	for (int j = 0; j < this->sys->numVars(); j++) t[j] = qq[j] - k[1][j] + 2.0*k[2][j];
	this->sys->der_func(t,dq);
	for (int j = 0; j < this->sys->numVars(); j++) k[3][j] = step*dq[j];
	// Compute k5
	for (int j = 0; j < this->sys->numVars(); j++)
		t[j] = qq[j] + (7.0/27.0)*k[0][j] + (10.0/27.0)*k[1][j] + (1.0/27.0)*k[3][j];
	this->sys->der_func(t,dq);
	for (int j = 0; j < this->sys->numVars(); j++) k[4][j] = step*dq[j];
	// Compute k6
	for (int j = 0; j < this->sys->numVars(); j++)
		t[j] = qq[j] + (28.0/625.0)*k[0][j] - 0.2*k[1][j] + (546.0/625.0)*k[2][j]
			+ (54.0/625.0)*k[3][j] - (378.0/625.0)*k[4][j];
	this->sys->der_func(t,dq);
	for (int j = 0 ; j < this->sys->numVars(); j++) k[5][j] = step*dq[j];
	// Compute next state and the approximate error
	double err = 0.0;
	for (int j = 0; j < this->sys->numVars(); j++)
	{
		// Next state
		qq[j] += (1.0/24.0)*k[0][j] + (5.0/48.0)*k[3][j] + 
			(27.0/56.0)*k[4][j] + (125.0/336.0)*k[5][j];
		// Componennt wise maximum of the approximate error
		err = std::max(err,
				fabs(k[0][j]/8.0+2.0*k[2][j]/3.0+k[3][j]/16.0-27.0*k[4][j]/56.0
					-125.0*k[5][j]/336.0));
	}
	// Return the error
	return err;
}

} // end of namespace
#endif

