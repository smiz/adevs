#ifndef _LengthMonitor_h_
#define _LengthMonitor_h_
#include "adevs.h"

/**
The LengthMonitor observes the oven conveyor and produces start and stop signals
based on the queue length.
*/
class LengthMonitor: public adevs::Atomic<adevs::PortValue<double> >
{
	public:
		// Input port for items that will be queued
		static const int in;
		// Output port for departing items
		static const int out;
		// Full and not full (not full = 1.0, full = 0.0)
		static const int status;
		// Constructor
		LengthMonitor(int queue_limit);
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
		~LengthMonitor(){}

	private:
		// Maximum queue size
		const unsigned int max_q_size;
		// Size of the queue
		unsigned int queue_size;
		// Current phase 
		enum { FULL, NOTFULL, SWITCH } phase;
};

#endif
