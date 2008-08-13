#ifndef _OvenControl_h_
#define _OvenControl_h_
#include "adevs.h"

/**
Observes the length monitor and cleaner signals, and turns these into
a sequence of on-off events for the mixer.
*/
class OvenControl: public adevs::Atomic<adevs::PortValue<double> >
{
	public:
		// Queue status (1 = ok, 0 = full)
		static const int q_status;
		// Cleaner status (1 = ok, 0 = cleaning)
		static const int cleaner_status;
		// Mixer switch (run = 1.0, stop = 0.0)
		static const int status;
		// Constructor
		OvenControl();
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
		~OvenControl(){}

	private:
		typedef enum { WORK, CLEAN, FULL, NOTFULL } Phase;
		Phase oven[2];
		Phase q[2];
};

#endif
