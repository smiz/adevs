#ifndef _Cleaner_h_
#define _Cleaner_h_
#include "adevs.h"

/**
The Cleaner observes the oven conveyor and begins a self clean cycles whenver
a predetermined number of cookies are produced.
*/
class Cleaner: public adevs::Atomic<adevs::PortValue<double> >
{
	public:
		// Input for queued dough balls
		static const int in;
		// Input port for cookies that are produced
		static const int out;
		// Clean and not run (run = 1.0, clean = 0.0)
		static const int status;
		// Constructor
		Cleaner(int cookie_limit, double burn_duration);
		// State transition functions
		void delta_int();
		void delta_ext(double e, const adevs::Bag<adevs::PortValue<double> >& xb);
		void delta_conf(const adevs::Bag<adevs::PortValue<double> >& xb);
		// Output function
		void output_func(adevs::Bag<adevs::PortValue<double> >& yb);
		// Time advance function
		double ta();
		// Garbage collection function - not needed in this example
		void gc_output(adevs::Bag<adevs::PortValue<double> >& g){}
		// Destructor
		~Cleaner(){}

	private:
		// Maximum number of cookies
		const unsigned int cookie_limit;
		// Self cleaning duration
		const double clean_duration;
		// Number of produced since the last cleaning
		unsigned int cookie_count;
		// Number of items in the queue
		unsigned int q_size;
		// Current phase 
		enum { WORK, CLEAR, CLEAN } phase;
};

#endif
