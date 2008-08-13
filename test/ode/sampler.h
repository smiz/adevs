#ifndef _sampler_h_
#define _sampler_h_
#include "adevs.h"
#include <iostream>

/**
Sampling function for continuous system simulator testing.
All input and output is through port 0
*/
class sampler: public adevs::Atomic<adevs::PortValue<double> >
{
	public:
		sampler(double dt):
		adevs::Atomic<adevs::PortValue<double> >(),
		dt(dt),
		sigma(dt),
		t(0.0)
		{
		}
		void delta_int(){ t += sigma; sigma = dt; }
		void delta_ext(double e, const adevs::Bag<adevs::PortValue<double> >& xb)
		{
			sigma -= e;
			t += e;
			std::cout << t << " ";
			adevs::Bag<adevs::PortValue<double> >::const_iterator iter;
			for (iter = xb.begin(); iter != xb.end(); iter++)
			{
				std::cout << (*iter).value << " ";
			}
			std::cout << std::endl; 
		}
		void delta_conf(const adevs::Bag<adevs::PortValue<double> >& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		double ta() { return sigma; }
		void output_func(adevs::Bag<adevs::PortValue<double> >& yb) 
		{ 
			adevs::PortValue<double> event(0,0.0);
			yb.insert(event); 
		}
		void gc_output(adevs::Bag<adevs::PortValue<double> >&){}
	private:
		const double dt;
		double sigma, t;
};

#endif
