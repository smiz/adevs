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
		s(0,0.0)
		{
		}
		void delta_int() 
		{
			assert(false);
		}
		void delta_ext(double e, const adevs::Bag<PortValue>& x) 
		{
			s.t += e;
			s.count += x.size();
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
			return DBL_MAX;
		}
		void gc_output(adevs::Bag<PortValue>& g)
		{
			assert(g.size() == 0);
		}
		~counter()
		{
		}
		void printState(void *data)
		{
			state_t* state = &s;
			if (data != NULL) state = (state_t*)data;
			printf("Count is %d @ %d\n",state->count,(int)(state->t));
		}
		void* save_state()
		{
			return new state_t(s);
		}
		void restore_state(void* data)
		{
			s = *((state_t*)data);
		}
		void gc_state(void* data)
		{
			delete (state_t*)data;
		}

	private:	
		struct state_t
		{
			state_t(int count, double t):
				count(count),
				t(t)
			{
			}
			state_t(const state_t& s):
				count(s.count),
				t(s.t)
			{
			}
			int count;
			double t;
		};
		state_t s;
};

const int counter::in = 0;

#endif
