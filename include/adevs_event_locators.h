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
		enum Mode { INTERPOLATE, BISECTION, DISCONTINUOUS };
		/**
		 * Create an event locator that will act on the supplied ode system.
		 * @param sys The system of equations to solve
		 * @param err_tol Threshold for the zero crossing function that will
		 * trigger an event detect.
		 * @param mode The event localization method to be used
		 */
		event_locator_impl(ode_system<X>* sys, double err_tol, Mode mode);
		/// Destructor leaves the ode_system object intact
		~event_locator_impl();
		/**
		 * Location discrete events between now and now + h.
		 * @param events This array contains one entry for each zero crossing
		 * function. An entry is true of that function is within err_tol of
		 * zero and false otherwise.
		 * @param qstart The continuous variable state vector now.
		 * @param qend The continuous value state vector when the event
		 * is detected or at now+h if no event is detected.
		 * @param solver The ode solver to use when computing continuous
		 * state vector values.
		 * @param h The interval of time over which to look for events.
		 * @return true if at least one event was found and false otherwise.
		 */
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
	while (this->sys->numEvents() > 0)
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
				if (
						// End condition when z is continuous
						((mode != DISCONTINUOUS) && (fabs(z[1][i]) <= err_tol)) ||
						// End condition when z is discontinuous
						((mode == DISCONTINUOUS) && (h <= err_tol))
					)
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
			if (mode == BISECTION || mode == DISCONTINUOUS) h /= 2.0;
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
 * Locate events using the bisection method. Your z functions must be
 * continuous for this to work.
 */
template <typename X>
class bisection_event_locator:
	public event_locator_impl<X>
{
	public:
		/// Create an event locator that implements the bisection search mode
		bisection_event_locator(ode_system<X>* sys, double err_tol):
			event_locator_impl<X>(
					sys,err_tol,event_locator_impl<X>::BISECTION){}
};

/**
 * Locate events using linear interpolation. Your z functions must be
 * continuous for this to work.
 */
template <typename X>
class linear_event_locator:
	public event_locator_impl<X>
{
	public:
		/// Create an event locator that implements the linear interpolation search mode
		linear_event_locator(ode_system<X>* sys, double err_tol):
			event_locator_impl<X>(
					sys,err_tol,event_locator_impl<X>::INTERPOLATE){}
};

/**
 * Locate events using bisection assuming discontinuous z functions.
 */
template <typename X>
class discontinuous_event_locator:
	public event_locator_impl<X>
{
	public:
		/**
		 * Create an event locator that implements a bisection search while
		 * assuming the z functions are not continuous.
		 */
		discontinuous_event_locator(ode_system<X>* sys, double err_tol):
			event_locator_impl<X>(
					sys,err_tol,event_locator_impl<X>::DISCONTINUOUS){}
};

/**
 * This event locator is for models that have no state events. Its find_events
 * method simply returns false.
 */
template <typename X> class null_event_locator:
	public event_locator<X>
{
	public:
		null_event_locator():event_locator<X>(NULL){}
		~null_event_locator(){}
		bool find_events(bool*, const double*, double*, ode_solver<X>*, double&)
		{
			return false;
		}
};

} // end of namespace 

#endif
