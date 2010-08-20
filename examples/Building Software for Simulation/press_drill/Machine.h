#ifndef _Machine_h_
#define _Machine_h_
#include "adevs.h"

class Machine: public adevs::Atomic<int>
{
	public:
		Machine(double tm):
			adevs::Atomic<int>(),
			tm(tm),p(0),sigma(tm){}
		void delta_int() { p--; sigma = tm; }
		void delta_ext(double e, const adevs::Bag<int>& xb)
		{
			if (p > 0) sigma -= e;
			else sigma = tm;
			for (adevs::Bag<int>::iterator iter = xb.begin();
					iter != xb.end(); iter++) 
				p += (*iter);
		}
		void delta_conf(const adevs::Bag<int>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(adevs::Bag<int>& yb) { yb.insert(1); }
		double ta()
		{
			if (p > 0) return sigma;
			else return DBL_MAX;
		}
		void gc_output(adevs::Bag<int>&){}
		double getSigma() const { return sigma; }
		int getParts() const { return p; }
	private:
		const double tm; // Machining time
		int p; // Number of parts in the bin
		double sigma; // Time to the next output
};

#endif
