/***************
Copyright (C) 2000-2006 by James Nutaro

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Bugs, comments, and questions can be sent to nutaro@gmail.com
***************/
#ifndef _adevs_rk45_h_
#define _adevs_rk45_h_
#include "adevs_dess.h"
#include <cmath>
#include <algorithm>

namespace adevs
{

/**
<p>This class is deprecated. Use the Hybrid class instead.
<p>This class can be used to simulate a set of ordinary differential
equations with state and time events. An adaptive fourth/fifth
order Runge-Kutta integration scheme is used to advance to solution,
and interval halving is used for
event detection. Truncation errors are estimated for the fourth
order scheme, but the solution is advanced with the fifth order scheme.
The inf-norm is used in the error tolerance test.
*/
template <class X> class rk45: public DESS<X>
{
	public:
		/**
		The constructor requires the number of continuous state variables,
		the maximum allowed integration time step, maximum truncation error
		tolerance, and the number of zero crossing
		functions in this system. An optional event tolerance sets the absolute
		time by which events can be missed (default is 1E-12).
		*/
		rk45(int num_state_vars, double h_max, double err_tol,
				int zero_crossing_funcs, double event_tol = 1E-12);
		/**
		Initialize the ith state variable
		*/
		void init(int i, double q0) { q[i] = q0; }
		/**
		Get read-only access to the state variable array.
		*/
		const double* getStateVars() const { return q; }
		/**
		 * Get the number of items in the state variable array.
		 */
		int getNumStateVars() const { return num_state_vars; }
		/**
		Compute the derivative function using the supplied state variable values.
		The derivatives should be stored in dq.
		*/
		virtual void der_func(const double* q, double* dq) = 0;
		/**
		This function should fill the array z with the value of the zero crossing
		functions.  The z array will have a number of entries equal to the zero_crossing_funcs
		argument that was passed to the constructor. An event occurs when any of these
		functions are equal to zero.
		*/
		virtual void state_event_func(const double* q, double* z) = 0;
		/**
		This method is used to schedule time event.  The returned value is the
		time remaining until the next time event.
		*/
		virtual double time_event_func(const double* q) = 0;
		/**
		This function is called when an event occurs.  The event can be due to
		the zero crossing function reaching zero or because of an external 
		input. The continuous and discrete variables can be updated here.  The
		q array contains continuous state when the event occurs, and can be modified
		if desired.
		*/
		virtual void discrete_action(double* q, const Bag<X>& xb) = 0;
		/**
		This output function is evaluated when internal events occur. Output events
		should be placed into the bag yb.
		*/
		virtual void discrete_output(const double* q, Bag<X>& yb) = 0;
		/**
		This method is called immediately following every time step, and
		just prior and following every discrete action. It is meant to allow
		output variables to be recorded to file or log every time that the
		model state changes. You chould not change the simulation state as part
		of this method! It is non-const just for convenience - please be careful.
		This method does nothing by default.
		*/
		virtual void state_changed(const double* q){}
		// Implementation of the DESS evolve_func method
		void evolve_func(double h);
		// Implementation of the DESS next_event_func method
		double next_event_func(bool& is_event);
		// Implementation of the DESS discrete_action_func method
		void discrete_action_func(const Bag<X>& xb);
		// Implementation of the DESS dscrete_output_func method
		void discrete_output_func(Bag<X>& yb);
		// Implementation of the DESS state changed method
		void state_changed() { state_changed(q); }
		/// Destructor
		~rk45();

	private:
		// Maximum step size, trunc. err. tolerance, and
		// state event detection time tolerance
		const double h_max, err_tol, event_tol;
		// Scratch variables to implement the integration 
		// and event detection scheme
		double *q, *dq, *t, *k[6], *q_tmp, *es, *en;
		// Current time step selection
		double h_cur;
		// Number of state variables and level crossing functions
		int num_state_vars, zero_funcs;
		/*
		 * This makes an appropriate sized step and stores the
		 * result in qq. The method returns truncation error
		 * estimate.
		 */
		double ode_step(double *qq, double step);

		static int sgn(double x)
		{
			if (x < 0.0) return -1;
			else if (x > 0.0) return 1;
			else return 0;
		}
};

template <class X>
rk45<X>::rk45(int num_state_vars, double h_max, double err_tol, int zero_funcs, double event_tol):
DESS<X>(),
h_max(h_max),
err_tol(err_tol),
event_tol(event_tol),
num_state_vars(num_state_vars),
zero_funcs(zero_funcs)
{
	h_cur = h_max;
	q = new double[num_state_vars];
	dq = new double[num_state_vars];
	t = new double[num_state_vars];
	q_tmp = new double[num_state_vars];
	for (int i = 0; i < 6; i++)
		k[i] = new double[num_state_vars];
	en = new double[zero_funcs];
	es = new double[zero_funcs];
}

template <class X>
rk45<X>::~rk45()
{
	delete [] q;
	delete [] dq;
	delete [] t;
	delete [] q_tmp;
	for (int i = 0; i < 6; i++)
		delete [] k[i];
	delete [] es;
	delete [] en;
}

template <class X>
void rk45<X>::evolve_func(double h)
{
	ode_step(q,h);
}

template <class X>
double rk45<X>::next_event_func(bool& is_event)
{
	// Get the next time event
	double time_event = time_event_func(q);
	// Look for the next integrator step size
	h_cur = std::min(h_max,h_cur*1.1); // Try something a little bigger
	for (int i = 0; i < num_state_vars; i++) q_tmp[i] = q[i];
	while (ode_step(q_tmp,h_cur) > err_tol) // Shrink until the tolerance is met
	{
		h_cur *= 0.9;
		for (int i = 0; i < num_state_vars; i++) q_tmp[i] = q[i];
	}
	// Look for the next state event
	double sigma = h_cur;
	state_event_func(q,es);
	for (int i = 0; i < zero_funcs; i++) en[i] = es[i];
	while (true)
	{
		// Look for an event in the next time step
		for (int i = 0; i < num_state_vars; i++) q_tmp[i] = q[i];
		// This will update the h_cur variable to satisfy
		// the error tolerance. Therefore, the step size
		// selection for error will also be found during
		// the search for state events
		ode_step(q_tmp,sigma);
		state_event_func(q_tmp,en);
		// If no event, then return
		is_event = false;
		for (int i = 0; i < zero_funcs && !is_event; i++)
		{
			is_event = (sgn(es[i]) != sgn(en[i]));
		}
		if (!is_event) break;
		// Otherwise, halve the interval and try again
		sigma /= 2.0;
		// If t_event is small, then just return, its been found
		if (sigma < event_tol)
		{
			is_event = true;
			break;
		}
	}
	// Return the time until the next event
	if (sigma < time_event) 
	{
		return sigma;
	}
	is_event = true;
	return time_event;
}

template <class X>	
void rk45<X>::discrete_action_func(const Bag<X>& xb)
{
	discrete_action(q,xb);
}

template <class X>	
void rk45<X>::discrete_output_func(Bag<X>& yb)
{
	discrete_output(q,yb);
}

template <class X>	
double rk45<X>::ode_step(double*qq, double step)
{
	if (step == 0.0)
	{
		return 0.0;
	}
	// Compute k1
	der_func(qq,dq); 
	for (int j = 0; j < num_state_vars; j++)
		k[0][j] = step*dq[j];
	// Compute k2
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + 0.5*k[0][j];
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[1][j] = step*dq[j];
	// Compute k3
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + 0.25*(k[0][j]+k[1][j]);
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[2][j] = step*dq[j];
	// Compute k4
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] - k[1][j] + 2.0*k[2][j];
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[3][j] = step*dq[j];
	// Compute k5
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + (7.0/27.0)*k[0][j] + (10.0/27.0)*k[1][j] + (1.0/27.0)*k[3][j];
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[4][j] = step*dq[j];
	// Compute k6
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + (28.0/625.0)*k[0][j] - 0.2*k[1][j] + (546.0/625.0)*k[2][j]
			+ (54.0/625.0)*k[3][j] - (378.0/625.0)*k[4][j];
	der_func(t,dq);
	for (int j = 0 ; j < num_state_vars; j++)
		k[5][j] = step*dq[j];
	// Compute next state and maximum approx. error
	double err = 0.0;
	for (int j = 0; j < num_state_vars; j++)
	{
		qq[j] += (1.0/24.0)*k[0][j] + (5.0/48.0)*k[3][j] + 
			(27.0/56.0)*k[4][j] + (125.0/336.0)*k[5][j];
		err = std::max(err,
				fabs(k[0][j]/8.0+2.0*k[2][j]/3.0+k[3][j]/16.0-27.0*k[4][j]/56.0
					-125.0*k[5][j]/336.0));
	}
	return err;
}

} // end of namespace

#endif
