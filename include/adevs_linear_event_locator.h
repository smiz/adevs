#ifndef _adevs_linear_event_locator_h_
#define _adevs_linear_event_locator_h_
#include "adevs_hybrid.h"
#include <cmath>

namespace adevs
{

/**
 * This is a state event locator that uses linear interpolation
 * on successively smaller intervals to pinpoint events in time.
 */
template <typename X> class linear_event_locator:
	public event_locator<X>
{
	public:
		/**
		 * The locator will attempt to pinpoint events within err_tol of
		 * zero for each state event function: i.e., an event occurs
		 * at time t if |z(t)| < err_tol, where z is a state event
		 * function.
		 */
		linear_event_locator(ode_system<X>* sys, double err_tol);
		~linear_event_locator();
		bool find_events(bool* events, const double* qstart, double* qend,
				ode_solver<X>* solver, double& h);
	private:
		double *z[2]; // State events at the start and end of [0,h]
		const double err_tol; // Error tolerance
};

template <typename X>
linear_event_locator<X>::linear_event_locator(ode_system<X>* sys,
		double err_tol):
	event_locator<X>(sys),err_tol(err_tol)
{
	z[0] = new double[sys->numEvents()];
	z[1] = new double[sys->numEvents()];
}

template <typename X>
linear_event_locator<X>::~linear_event_locator()
{
	delete [] z[0]; delete [] z[1];
}

template <typename X>
bool linear_event_locator<X>::find_events(bool* events,
	const double* qstart, double* qend, ode_solver<X>* solver, double& h)
{
	// Look for events at the start of the interval
	this->sys->state_event_func(qstart,z[0]);
	for (int i = 0; i < this->sys->numEvents(); i++) {
		events[i] = fabs(z[0][i]) <= err_tol;
		// If an event was found, the event time is zero
		if (events[i]) h = 0.0;
	}
	// If an event was found at zero, put qstart in qend and return
	if (h == 0.0) {
		for (int i = 0; i < this->sys->numVars(); i++) qend[i] = qstart[i];
		return true;
	}
	// Look for events inside of the interval [0,h]
	for (;;) {
		double tguess = h;
		bool event_in_interval = false, found_event = false;
		this->sys->state_event_func(qend,z[1]);
		for (int i = 0; i < this->sys->numEvents(); i++) {
			if ((events[i] = fabs(z[1][i]) <= err_tol)) found_event = true;
			else if (z[0][i]*z[1][i] < 0.0) {
				double tcandidate = z[0][i]*h/(z[0][i]-z[1][i]);
				if (tcandidate < tguess) tguess = tcandidate;
				event_in_interval = true;
			}
		}
		// Calculate a new solution at tguess if an event was found
		if (event_in_interval) {
			h = tguess;
			for (int i = 0; i < this->sys->numVars(); i++)
				qend[i] = qstart[i];
			solver->advance(qend,h);
		}
		// Stop when an event is located or is not detected in the interval
		else return found_event;
	}
	// Will never reach this line
	return false;
}

} // end of namespace 

#endif
