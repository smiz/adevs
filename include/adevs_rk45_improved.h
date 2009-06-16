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
#ifndef _adevs_rk45_improved_h_
#define _adevs_rk45_improved_h_
#include "adevs_dess.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <cassert>

namespace adevs
{

/**
<p>This class can be used to simulate a set of ordinary differential
equations with state and time events. An adaptive fourth/fifth
order Runge-Kutta integration scheme is used to advance to solution.
Truncation errors are estimated for the fourth
order scheme, but the solution is advanced with the fifth order scheme.
The inf-norm is used in the error tolerance test.</p>
<p>This class is 'improved' over the rk45 class in the sense that its
algorithm for detecting state events converges more rapidly, giving (in general)
shorter simulation times. This comes with a cost: 1) the state event functions must
have first derivatives (i.e., can be successfully approximated with a line) and 2) 
you have to be very careful about event conditions. The algorithm looks for a change
from positive to negative (or negative to positive) in the sign of the event function. When the event is executed,
you must be absolutely certain that the sign of the event function won't switch again in
zero time. For instance, in a bouncing ball simulation with bouncing condition height < 0,
be sure to set the height to zero and velocity to a non-negative value when the event occurs.
If you don't the height may still be (slightly) less than zero
after the event and cause an infinite number of state changes (i.e., result in non-zeno behavior).
</p>
*/
template <class X> class rk45_improved: public DESS<X>
{
	public:
		/**
		The constructor requires the number of continuous state variables,
		the maximum allowed integration time step, maximum truncation error
		tolerance, and the number of zero crossing
		functions in this system. An optional event tolerance sets the absolute
		time by which events can be missed (default is 1E-12).
		*/
		rk45_improved(int num_state_vars, double h_max, double err_tol,
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
		if desired. The state event list tells you which state event functions 
		(if any) triggered the action. The last entry (the index is equal to
		the number of zero functions) in the event flag
		list is true if this action coincides with a time event. Note that this
		means there are a number of positions in the list equal to 
		the number of zero functions passed to the constructor (your real zero
		functions) plus an extra one added by the simulator (to indicate
		time events).
		*/
		virtual void discrete_action(double* q, const Bag<X>& xb, const bool* event_flags) = 0;
		/**
		This output function is evaluated when internal events occur. Output events
		should be placed into the bag yb.
		*/
		virtual void discrete_output(const double* q, Bag<X>& yb, const bool* event_flags) = 0;
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
		~rk45_improved();

	private:
		// Maximum step size, trunc. err. tolerance, and
		// state event detection time tolerance
		const double h_max, err_tol, event_tol;
		// Scratch variables to implement the integration 
		// and event detection scheme
		double *q, *dq, *t, *k[6], *q_tmp, *es, *en;
		// Event indicator flags
		bool* event_indicator;
		// Current time step selection
		double h_cur;
		// Does q_tmp hold the ODE solution at h_cur?
		bool keep_q_tmp;
		// Number of state variables and level crossing functions
		const int num_state_vars, zero_funcs;
		/*
		 * This makes an appropriate sized step and stores the
		 * result in qq. The method returns truncation error
		 * estimate.
		 */
		double ode_step(double *qq, double step);
};

template <class X>
rk45_improved<X>::rk45_improved(int num_state_vars, double h_max, double err_tol, int zero_funcs, double event_tol):
DESS<X>(),
h_max(h_max),
err_tol(err_tol),
event_tol(event_tol),
h_cur(h_max),
keep_q_tmp(false),
num_state_vars(num_state_vars),
zero_funcs(zero_funcs)
{
	q = new double[num_state_vars];
	dq = new double[num_state_vars];
	t = new double[num_state_vars];
	q_tmp = new double[num_state_vars];
	for (int i = 0; i < 6; i++)
		k[i] = new double[num_state_vars];
	en = new double[zero_funcs];
	es = new double[zero_funcs];
	event_indicator = new bool[zero_funcs+1];
}

template <class X>
rk45_improved<X>::~rk45_improved()
{
	delete [] q;
	delete [] dq;
	delete [] t;
	delete [] q_tmp;
	for (int i = 0; i < 6; i++)
		delete [] k[i];
	delete [] es;
	delete [] en;
	delete [] event_indicator;
}

template <class X>
void rk45_improved<X>::evolve_func(double h)
{
	// If this is an internal event 
	if (h == h_cur)
	{
		// if q_tmp is ok, then just copy q_tmp to q
		if (keep_q_tmp)
		{
			for (int i = 0; i < num_state_vars; i++)
				q[i] = q_tmp[i];
		} 
		// otherwise advance the solution
		else ode_step(q,h);
	}
	// If this is an external event, then check for state events and
	// advance the solution
	else
	{
		// This is not a time event
		event_indicator[zero_funcs] = false;
		// Calculate the state event function at q
		state_event_func(q,es);
		// Advance the solution
		ode_step(q,h); 
		// Calculate the state event function now
		state_event_func(q,en);
		// Look for zero crossings in the internval
		for (int i = 0; i < zero_funcs; i++)
		{
			event_indicator[i] = (es[i]*en[i] < 0.0) || (fabs(en[i]) <= event_tol);
		}
	}
	// Invalidate q_tmp	
	keep_q_tmp = false;
}

template <class X>
double rk45_improved<X>::next_event_func(bool& is_event)
{
	// q_tmp will hold the solution at h_cur when this method returns
	keep_q_tmp = true;
	// Get the next time event
	double time_event = time_event_func(q);
	/* Look for the largest allowable integration step */
	h_cur *= 1.2; // Try a larger step size this time through
	if (h_cur > h_max) h_cur = h_max; // Limit to h_max
	if (h_cur > time_event) h_cur = time_event; // Limit to the next event time
	for ( ; ; )
	{
		// Advance the solution by h_cur
		for (int i = 0; i < num_state_vars; i++) q_tmp[i] = q[i]; 
		double err = ode_step(q_tmp,h_cur); 
		if (err <= err_tol) break; // Check the error, exit if ok
		// Reduce the step size otherwise
		double h_next = 0.8*pow(err_tol*h_cur*h_cur*h_cur*h_cur*h_cur,0.2)/fabs(err);
		if (h_next >= h_cur) h_next = 0.8*h_cur;
		h_cur = h_next;
	}
	// q_tmp now stores the state variables at the end of the time step
	// and q has the variables at the start of the time step. Now we
	// look for state events. The array es contains the state event
	// functions at the start of the interval.
	state_event_func(q,es); 
	// Look for the next zero crossing
	while(true)
	{
		// Compute the state event values at the end of the interval
		state_event_func(q_tmp,en);
		// Look for a zero-crossing 
		bool found_state_event = false;
		double h_next = h_cur;
		for (int i = 0; i < zero_funcs; i++)
		{
			bool sign_change = (es[i]*en[i] < 0.0);
			bool tolerance_met = event_indicator[i] = (fabs(en[i]) <= event_tol);
			if (tolerance_met) found_state_event = true;
			// Estimate the time to cross zero and remember the
			// smallest such time (if we actual found an event, then h_cur
			// is the crossing time).
			if (sign_change && !tolerance_met)
			{
				double t_cross = h_cur*es[i]/(es[i]-en[i]);
				assert(t_cross >= 0.0);
				if (t_cross < h_next) h_next = t_cross;
			}
		}
		// If the next event time is equal to the current event time, then
		// we found the next event crossing or there wasn't one.
		if (h_next == h_cur)
		{
			// Is this a time event?
			event_indicator[zero_funcs] = (h_next >= time_event); 
			// Are there any state or time events?
			is_event = found_state_event || event_indicator[zero_funcs];
			// Done, return the next event time
			return h_cur;
		}
		// If a crossing was found adjust the time step and try again
		assert(h_next < h_cur);
		h_cur = h_next;
		for (int i = 0; i < num_state_vars; i++) q_tmp[i] = q[i];
		ode_step(q_tmp,h_cur);
	}
}

template <class X>	
void rk45_improved<X>::discrete_action_func(const Bag<X>& xb)
{
	// Reset the integrator step size and invalidate q_tmp
	h_cur = h_max;
	keep_q_tmp = false;
	// Compute the discrete action
	discrete_action(q,xb,event_indicator);
}

template <class X>	
void rk45_improved<X>::discrete_output_func(Bag<X>& yb)
{
	discrete_output(q,yb,event_indicator);
}

template <class X>	
double rk45_improved<X>::ode_step(double*qq, double step)
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
