#ifndef __counter_h_
#define __counter_h_
#include "adevs.h"
#include "object.h"
#include <cstdio>
#include <cassert>

class counter: public adevs::Atomic<PortValue>
{
	public:

		static const int in;

		counter():
		adevs::Atomic<PortValue>(),
		count(0),
		sigma(DBL_MAX),
		t(0.0)
		{
		}
		void delta_int() 
		{
			assert(false);
		}
		void delta_ext(double e, const adevs::Bag<PortValue>& x) 
		{
			t += e;
			count += x.size();
			printf("Count is %d @ %d\n",count,(int)t);
			sigma = DBL_MAX; 
		} 
		void delta_conf(const adevs::Bag<PortValue>&)
		{
			assert(false);
		}
		void output_func(adevs::Bag<PortValue>&)
		{
			assert(false);
		}
		double ta()
		{
			return sigma;
		}
		void gc_output(adevs::Bag<PortValue>& g)
		{
			assert(g.size() == 0);
		}
		~counter()
		{
		}
	private:	
		int count;
		double sigma, t;
};

const int counter::in = 0;

#endif
