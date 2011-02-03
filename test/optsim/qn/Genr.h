#ifndef _Sw_h_
#define _Sw_h_
#include "adevs.h"
#include <cstdlib>
#include <iostream>

class Genr: public adevs::Atomic<int>
{
	public:
		Genr(int freq):adevs::Atomic<int>(),
			events(0),period(1.0/(double)freq)
		{
			srandom(1);
			next = random();
		}
		void delta_int(){ next = random(); events++; }
		void delta_ext(double, const adevs::Bag<int>&){}
		void delta_conf(const adevs::Bag<int>&){}
		double ta() { return period; }
		void output_func(adevs::Bag<int>& yb) { yb.insert(next); }
		void gc_output(adevs::Bag<int>&){}
		double lookahead() { return DBL_MAX; }
	private:
		unsigned int events;
		long int next;
		const double period;
};

class Collector: public adevs::Atomic<int>
{
	public:
		Collector():adevs::Atomic<int>(),events(0){}
		void delta_int(){}
		void delta_ext(double, const adevs::Bag<int>& xb){ events += xb.size(); }
		void delta_conf(const adevs::Bag<int>&){}
		double ta() { return DBL_MAX; }
		void output_func(adevs::Bag<int>&){}
		void gc_output(adevs::Bag<int>&){}
		double lookahead() { return DBL_MAX; }
	private:
		unsigned int events;
};

#endif
