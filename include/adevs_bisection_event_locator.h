#ifndef _adevs_bisection_event_locator_h_
#define _adevs_bisection_event_locator_h_
#include "adevs_hybrid.h"
#include <cmath>

namespace adevs
{
/**
 * This is a state event locator that uses bisection to find
 * an event in time. The specific algorithm used here is
 * guaranteed to return a timestep that places the state
 * variables on the opposite side of the event surface.
 * That is, the sign of the function is guaranteed to change.
 */
template <typename X> class bisection_event_locator:
	public event_locator<X>
{
	public:
		/**
		 * The locator will attempt to pinpoint events within err_tol of
		 * zero on the opposite side of the event surface. An event occurs
		 * at time t' if |z(t')| < err_tol and z(t)z(t') < 0, where t
		 * is the time at the start of the search.
		 */
		bisection_event_locator(ode_system<X>* sys, double err_tol);
		~bisection_event_locator();
		bool find_events(bool* events, const double* qstart, double* qend,
				ode_solver<X>* solver, double& h);
	private:
		double *z[2]; // State events at the start and end of [0,h]
		const double err_tol; // Error tolerance
};

template <typename X>
bisection_event_locator<X>::bisection_event_locator(ode_system<X>* sys,
		double err_tol):
	event_locator<X>(sys),err_tol(err_tol)
{
	z[0] = new double[sys->numEvents()];
	z[1] = new double[sys->numEvents()];
}

template <typename X>
bisection_event_locator<X>::~bisection_event_locator()
{
	delete [] z[0]; delete [] z[1];
}

template <typename X>
bool bisection_event_locator<X>::find_events(bool* events,
	const double* qstart, double* qend, ode_solver<X>* solver, double& h)
{
	// Calculate the state event functions at the start of the interval
	this->sys->state_event_func(qstart,z[0]);
	// Look for events inside of the interval [0,h]
	for (;;)
	{
		bool event_in_interval = false, found_event = false;
		// Get the state event functions at the end of the interval
		this->sys->state_event_func(qend,z[1]);
		// Do any of the z functions change sign? Have we found an event
		for (int i = 0; i < this->sys->numEvents(); i++)
		{
			events[i] = false;
			if (z[1][i]*z[0][i] <= 0.0)
			{
				// There is an event at h
				if (fabs(z[1][i]) <= err_tol)
					events[i] = found_event = true; 
				// There is an event prior to h
				else event_in_interval = true;
			}
		}
		// Guess at a new h and calculate qend for that time
		if (event_in_interval)
		{
			h /= 2.0;
			for (int i = 0; i < this->sys->numVars(); i++)
				qend[i] = qstart[i];
			solver->advance(qend,h);
		}
		else return found_event;
	}
	// Will never reach this line
	return false;
}

} // end of namespace 

#endif
