#ifndef _mixer_h_
#define _mixer_h_
#include "adevs.h"
#include "adevs_rk4.h"
#include <iostream>
#include <cmath>

/**
This is a model of a dough mixing machine.  The machine can be
started and stopped, and produces a new ball of dough whenever another
Q units of dough are produced.
*/
class Mixer: public adevs::rk4<adevs::PortValue<double> >
{
	public:
		// Output port for continuous variable
		static const int dough;
		// Output port for producing dough balls (output value is the ball size Q)
		static const int dough_ball;
		// Input port for getting start (value is 1.0) and stop (value is 0.0) events
		static const int knob;
		// Constructor. Begins with zero dough initially.
		Mixer();
		// Derivative of the dough production function.
		void der_func(const double* q, double *dq);
		// Zero crossing function for finding dough ball production events.
		void state_event_func(const double *q, double* z);
		// Time event scheduler - used to sample the continuous output
		double time_event_func(const double* q);
		// Discrete change function for handling state and input events
		void discrete_action(double* q, const adevs::Bag<adevs::PortValue<double> >& xb);
		// Dough ball output function.
		void discrete_output(const double* q, adevs::Bag<adevs::PortValue<double> >& yb);
		// Adevs garbage collection function - not needed in this example
		void gc_output(adevs::Bag<adevs::PortValue<double> >& g){}

	private:
		// Dough quantity at last production time
		double ql;
		// Is the machine working or idle?
		bool active;
		// Dough ball size
		const double Q; 
		// Needs an immediate sample?
		bool immediate;
		// Next sample time
		double sample;
};

#endif

