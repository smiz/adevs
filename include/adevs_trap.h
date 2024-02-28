/**
 * Copyright (c) 2023, James Nutaro
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
#ifndef _adevs_trap_h_
#define _adevs_trap_h_
#include <cmath>
#include <cassert>
#include <algorithm>
#include "adevs_hybrid.h"

namespace adevs
{

/**
 * This is the second order accurate trapezoidal method. You must supply
 * a jacobian to use this integration method. 
 */
template <typename X> class trap:
	public ode_solver<X>
{
	public:
		/**
		 * Create an integrator that will use the given
		 * maximum step size. The tolerance is used to
		 * terminate the newton iteration used at each
		 * integration step.
		 * @param sys The system to solve
		 * @param err_tol Truncation error limit. Global error is approximately steps times this.
		 * @param h_max Maximum allowed step size
		 * @param iter_limit N*iter_limit is the convergence criteria for the newton solver.
		 */
		trap(ode_system<X>* sys, double err_tol, double h_max, double iter_limit = 1E-12);
		/// Destructor
		~trap();
		double integrate(double* q, double h_lim);
		void advance(double* q, double h);
	private:
		double *q_iter[2]; // Solutions to use in newton iteration
		double *k; // Fixed term in newton iteration
		double *dq; // Derivatives at guess
		double *J; // jacobian
		int *ipiv; // pivot array for LAPACK
		const double err_tol;
		const double h_max; // Maximum time step
		const double tol;
};

template <typename X>
trap<X>::trap(ode_system<X>* sys, double err_tol, double h_max, double iter_limit):
	ode_solver<X>(sys),err_tol(err_tol),h_max(h_max),tol(iter_limit)
{
	q_iter[0] = new double[sys->numVars()];
	q_iter[1] = new double[sys->numVars()];
	dq = new double[sys->numVars()];
	k = new double[sys->numVars()];
	J = new double[sys->numVars()*sys->numVars()];
	ipiv = new int[sys->numVars()];
}

template <typename X>
trap<X>::~trap()
{
	delete [] q_iter[0];
	delete [] q_iter[1];
	delete [] dq;
	delete [] k;
	delete [] J;
	delete [] ipiv;
}

template <typename X>
void trap<X>::advance(double* q, double h)
{
	double dt;
	while ((dt = integrate(q,h)) < h) h -= dt;
}

// Linear solver from LAPAKC
extern "C" {
	void dgesv_(int*,int*,double*,int*,int*,double*,int*,int*);
};

template <typename X>
double trap<X>::integrate(double* q, double h_lim)
{
	double* tmp;
	double err;
	int info;
	int NRHS = 1;
	int N = this->sys->numVars();
	// Pick the step size step size
	double h = std::min<double>(h_max,h_lim);
	// Get the derivatives at the current point
	this->sys->der_func(q,dq);
	// Fixed term in the trapezoidal integration rule
	for (int i = 0; i < N; i++) {
		q_iter[0][i] = k[i] = q[i]+(h/2.0)*dq[i];
	}
	// Calculate the first guess assuming that J is the 
	// transition matrix of a linear system. This will either
	// get us to the next step if the system is, in fact,
	// linear or get us close enough to start iterating.
	this->sys->get_jacobian(q,J);
	for (int i = 0; i < N*N; i++)
		J[i] *= -h/2.0;
	for (int i = 0; i < N; i++)
		J[i*(N+1)] += 1.0;
	dgesv_(&N,&NRHS,J,&N,ipiv,q_iter[0],&N,&info);
	// Use Newton's method to solve the fixed point problem
	for (;;) {
		// Get the derivatives at the current point
		this->sys->der_func(q_iter[0],dq);
		// Get the jacobian at the current guess
		this->sys->get_jacobian(q_iter[0],J);
		// Get the A matrix for the linear solver
		for (int i = 0; i < N*N; i++)
			J[i] *= h/2.0;
		for (int i = 0; i < N; i++)
			J[i*(N+1)] -= 1.0;
		// Calculate the b vector
		for (int i = 0; i < N; i++) {
			q_iter[1][i] = k[i]+(h/2.0)*dq[i]-q_iter[0][i];
		}
		// Solve for the new guess
		dgesv_(&N,&NRHS,J,&N,ipiv,q_iter[1],&N,&info);
		assert(info == 0);
		for (int i = 0; i < N; i++) {
			q_iter[1][i] = q_iter[0][i]-q_iter[1][i];
		}
		// If the two solutions are close enough, then we are done
		err = fabs(q_iter[0][0]-q_iter[1][0]);
		for (int i = 1; i < N; i++)
			err += fabs(q_iter[0][i]-q_iter[1][i]);
		if (err < tol*N)
			break;
		// Swap solutions and repeat
		tmp = q_iter[1];
		q_iter[1] = q_iter[0];
		q_iter[0] = tmp;
	}
	// Put the trial solution in q and return the selected step size
	for (int i = 0; i < N; i++)
		q[i] = q_iter[1][i];
	return h;
}

} // end of namespace
#endif
