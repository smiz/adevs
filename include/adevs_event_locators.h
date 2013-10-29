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
#ifndef _adevs_event_locators_h_
#define _adevs_event_locators_h_
#include "adevs_hybrid.h"
#include <cmath>
#include <iostream>

namespace adevs
{

/**
 * This is a state event locator that uses either bisection or
 * linear interpolation to pinpoints events in time.
 */
template <typename X> class event_locator_impl:
	public event_locator<X>
{
	public:
		/**
		 * The locator will attempt to pinpoint events within err_tol of
		 * zero for each state event function; i.e., an event occurs
		 * at the first instant t' >= t where z(t)*z(t') <= 0 and
		 * |z(t')| < err_tol.
		 */
		enum Mode { INTERPOLATE, BISECTION };
		event_locator_impl(ode_system<X>* sys, double err_tol, Mode mode);
		~event_locator_impl();
		bool find_events(bool* events, const double* qstart, double* qend,
				ode_solver<X>* solver, double& h);
	private:
		double *z[2]; // State events at the start and end of [0,h]
		const double err_tol; // Error tolerance
		Mode mode;

		int sign(double x) const
		{
			if (x < 0.0) return -1;
			else if (x > 0.0) return 1;
			else return 0;
		}
};

template <typename X>
event_locator_impl<X>::event_locator_impl(ode_system<X>* sys,
		double err_tol, Mode mode):
	event_locator<X>(sys),
	err_tol(err_tol),
	mode(mode)
{
	z[0] = new double[sys->numEvents()];
	z[1] = new double[sys->numEvents()];
}

template <typename X>
event_locator_impl<X>::~event_locator_impl()
{
	delete [] z[0]; delete [] z[1];
}

template <typename X>
bool event_locator_impl<X>::find_events(bool* events,
	const double* qstart, double* qend, ode_solver<X>* solver, double& h)
{
	// Calculate the state event functions at the start 
	// of the interval
	this->sys->state_event_func(qstart,z[0]);
	// Look for the first event inside of the interval [0,h]
	for (;;)
	{
		double tguess = h;
		bool event_in_interval = false, found_event = false;
		this->sys->state_event_func(qend,z[1]);
		// Do any of the z functions change sign? Have we found an event?
		for (int i = 0; i < this->sys->numEvents(); i++)
		{
			events[i] = false;
			if (sign(z[1][i]) != sign(z[0][i]))
			{
				// Event at h > 0
				if (fabs(z[1][i]) <= err_tol)
				{
					events[i] = found_event = true; 
				}
				// There is an event in (0,h)
				else 
				{
					if (mode == INTERPOLATE)
					{
						double tcandidate = z[0][i]*h/(z[0][i]-z[1][i]);
						// Don't let the step size go to zero
						if (tcandidate < h/4.0) tcandidate = h/4.0;
						if (tcandidate < tguess) tguess = tcandidate;
					}
					event_in_interval = true;
				}
			}
		}
		// Guess at a new h and calculate qend for that time
		if (event_in_interval)
		{
			if (mode == BISECTION) h /= 2.0;
			else h = tguess;
			for (int i = 0; i < this->sys->numVars(); i++)
				qend[i] = qstart[i];
			solver->advance(qend,h);
		}
		else return found_event;
	}
	// Will never reach this line
	return false;
}

/**
 * Locate events using the bisection method.
 */
template <typename X>
class bisection_event_locator:
	public event_locator_impl<X>
{
	public:
		bisection_event_locator(ode_system<X>* sys, double err_tol):
			event_locator_impl<X>(
					sys,err_tol,event_locator_impl<X>::BISECTION){}
};

/**
 * Locate events using linear interpolation.
 */
template <typename X>
class linear_event_locator:
	public event_locator_impl<X>
{
	public:
		linear_event_locator(ode_system<X>* sys, double err_tol):
			event_locator_impl<X>(
					sys,err_tol,event_locator_impl<X>::INTERPOLATE){}
};

} // end of namespace 

#endif
