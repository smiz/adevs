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
#ifndef _adevs_rk4_h_
#define _adevs_rk4_h_
#include "adevs_dess.h"

namespace adevs
{

/**
<p>This class is deprecated, use the Hybrid class instead.
<p>This class can be used to simulate a set of ordinary differential
equations with state and time events. This classes uses a fourth
order Runge-Kutta integration scheme and interval halving for
event detection.
*/
template <class X> class rk4: public DESS<X>
{
	public:
		/**
		The constructor requires the number of continuous state variables and
		the maximum allowed integration time step. The number of zero crossing
		functions in this system are supplied with the last argument (the
		default value is 1).
		*/
		rk4(int num_state_vars, double h_max, int zero_crossing_funcs = 1);
		/**
		Initialize the ith state variable
		*/
		void init(int i, double q0) { q[i] = q0; }
		/**
		Get read-only access to the state variable array.
		*/
		const double* getStateVars() const { return q; }
		/**
		 * Get the number of state variables.
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
		This method is called initially and whenever a state change
		due to an event or integration step occurs. It is meant to
		be used for recording the state trajectory of the model.
		*/
		virtual void state_changed(const double* q){};
		// Implementation of the DESS evolve_func method
		void evolve_func(double h);
		// Implementation of the DESS next_event_func method
		double next_event_func(bool& is_event);
		// Implementation of the DESS discrete_action_func method
		void discrete_action_func(const Bag<X>& xb);
		// Implementation of the DESS dscrete_output_func method
		void discrete_output_func(Bag<X>& yb);
		// Implementation of the DESS state_changed method
		void state_changed() { state_changed(q); }
		/// Destructor
		~rk4();

	private:
		const double h_max;
		double *q, *dq, *t, *k[4], *q_tmp, *es, *en;
		int num_state_vars, zero_funcs;

		void ode_step(double *qq, double step);

		static int sgn(double x)
		{
			if (x < 0.0) return -1;
			else if (x > 0.0) return 1;
			else return 0;
		}
};

template <class X>
rk4<X>::rk4(int num_state_vars, double h_max, int zero_funcs):
DESS<X>(),
h_max(h_max),
num_state_vars(num_state_vars),
zero_funcs(zero_funcs)
{
	q = new double[num_state_vars];
	dq = new double[num_state_vars];
	t = new double[num_state_vars];
	q_tmp = new double[num_state_vars];
	for (int i = 0; i < 4; i++)
		k[i] = new double[num_state_vars];
	en = new double[zero_funcs];
	es = new double[zero_funcs];
}

template <class X>
rk4<X>::~rk4()
{
	delete [] q;
	delete [] dq;
	delete [] t;
	delete [] q_tmp;
	for (int i = 0; i < 4; i++)
		delete [] k[i];
	delete [] es;
	delete [] en;
}

template <class X>
void rk4<X>::evolve_func(double h)
{
	ode_step(q,h);
}

template <class X>
double rk4<X>::next_event_func(bool& is_event)
{
	// Look for event crossings
	double time_event = time_event_func(q);
	double sigma = h_max;
	state_event_func(q,es);
	for (int i = 0; i < zero_funcs; i++) en[i] = es[i];
	while (true)
	{
		// Look for an event in the next time step
		for (int i = 0; i < num_state_vars; i++)
		{
			q_tmp[i] = q[i];
		}
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
		if (sigma < 1E-12)
		{
			is_event = true;
			break;
		}
	}
	if (sigma < time_event) 
	{
		return sigma;
	}
	is_event = true;
	return time_event;
}

template <class X>	
void rk4<X>::discrete_action_func(const Bag<X>& xb)
{
	discrete_action(q,xb);
}

template <class X>	
void rk4<X>::discrete_output_func(Bag<X>& yb)
{
	discrete_output(q,yb);
}

template <class X>	
void rk4<X>::ode_step(double*qq, double step)
{
	if (step == 0.0)
	{
		return;
	}
	// Compute k1
	der_func(qq,dq); 
	for (int j = 0; j < num_state_vars; j++)
		k[0][j] = dq[j];
	// Compute k2
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + 0.5*step*k[0][j];
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[1][j] = dq[j];
	// Compute k3
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + 0.5*step*k[1][j];
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[2][j] = dq[j];
	// Compute k4
	for (int j = 0; j < num_state_vars; j++)
		t[j] = qq[j] + step*k[2][j];
	der_func(t,dq);
	for (int j = 0; j < num_state_vars; j++)
		k[3][j] = dq[j];
	// Compute next state
	for (int j = 0; j < num_state_vars; j++)
		qq[j] += step*(k[0][j] + 2.0*k[1][j] + 2.0*k[2][j] + k[3][j])/6.0;
}

} // end of namespace

#endif
