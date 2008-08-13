#ifndef _oven_h_
#define _oven_h_
#include "Monitor.h"
using namespace adevs;

const int Monitor::in = 0;
const int Monitor::out = 1;
const int Monitor::alert = 2;

Monitor::Monitor(int cookie_limit, int queue_limit, double clean_cycle_duration):
Atomic<PortValue<double> >(),
max_q_size(queue_limit),
clean_cycle(clean_cycle_duration),
cookie_limit(cookie_limit)
{
	num_cookies = 0;
	queue_size = 0;
	phase = RUNNING;
	sigma = DBL_MAX;
}

void Monitor::delta_int()
{
}

void Monitor::delta_ext(double e, const Bag<PortValue<double> >& xb)
{
	int prev_q_size = queue_size;
	Bag<PortValue<double> >::const_iterator iter;
	for (iter = xb.begin(); iter != xb.end(); iter++)
	{
		if ((*iter).port == in) queue_size++;
		else if ((*iter).port == out) { num_cookies++; queue_size--; }
		assert(queue_size <= max_q_size);
	}
	if (num_cookies >= cookie_limit)
	{
		phase = CLEAN;
		sigma = clean_cycle;
	}
	else if (phase == MONITOR && queue_size == max_q_size || prev_q_size == max_q_size)
	{
		sigma = 0.0;
	}
	else
	{
		sigma = DBL_MAX;
	}
}

		void delta_conf(const Bag<PortValue<double> >& xb);
		// Output function
		void output_func(Bag<PortValue<double> >& yb);
		// Time advance function
		double ta();
		// Garbage collection function - not needed in this example
		void gc_output(Bag<PortValue<double> >& g){}
		// Destructor
		~Monitor(){}

	private:
		// Maximum queue size
		const unsigned int max_q_size;
		// Clean cycle time
		const double clean_cycle;
		// Cookie limit between cleaning cycles
		const int cookie_limit;
		// Number of cookies processes
		int num_cookies;
		// Size of the queue
		int queue_size;
		// Current phase 
		enum { CLEAN, MONITOR } phase;
};

#endif
