#ifndef _oven_h_
#define _oven_h_
#include "adevs.h"
#include <list>

/**
A FIFO queue with server. The queue generates a stop event
when it become fall, and a start event with it ceases to be full.
Service time is equal to the size of the part.
*/
class Oven: public adevs::Atomic<adevs::PortValue<double> >
{
	public:
		// Input port for items that will be queued
		static const int in;
		// Output port for departing items
		static const int out;
		// Constructor
		Oven();
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
		~Oven(){}

	private:
		// Queue of parts
		std::list<double> q;
};

#endif
