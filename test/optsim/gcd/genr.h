#ifndef __genr_h_
#define __genr_h_
#include "adevs.h"
#include "object.h"
#include <cstdio>
#include <vector>
#include <cassert>

class genr: public adevs::Atomic<PortValue> 
{
	public:

		static const int stop;
		static const int start;
		static const int signal;

		genr(const std::vector<double>& pattern, 
		int iterations, bool active = true):
		adevs::Atomic<PortValue>(),
		pattern(pattern),
		active(active),
		init_state(active),
		iterations(iterations)
		{
			init();
		}
		genr(double period, int iterations, bool active = true):
		adevs::Atomic<PortValue>(),
		active(active),
		init_state(active),
		iterations(iterations)
		{
			pattern.push_back(period);
			init();
		}
		double lookahead()
		{
			double l = DBL_MAX;
			for (unsigned i = 0; i < pattern.size(); i++)
				if (pattern[i] < l) l = pattern[i];
			assert(l > 0.0);
			return l;
		}
		void init()
		{
			assignToLP(1);
			active = init_state;
			count = 1;
			if (!active) sigma = DBL_MAX;
			else
			{
				assert(pattern[0] >= 0.0);
		 		sigma = pattern[0];
			}
		}
		void delta_int()
		{
			if (count >= iterations) sigma = DBL_MAX;
			else
			{ 
				assert(pattern[count%pattern.size()] >= 0.0);
				sigma = pattern[count++%pattern.size()];
			}
		}
		void delta_ext(double, const adevs::Bag<PortValue>& x) 
		{
			sigma = DBL_MAX;
			active = false;
			adevs::Bag<PortValue>::const_iterator i;
			for (i = x.begin(); i != x.end(); i++)
			{
				if ((*i).port == start)
				{
					if (active == false)
					{
						active = true;
						count = 1;
						assert(pattern[0] >= 0.0);
						sigma = pattern[0];
						return;
					}
				}
			}
			num_stop_inputs++;
		}
		void delta_conf(const adevs::Bag<PortValue>& x)
		{
			delta_ext(ta(),x);
		}
		void output_func(adevs::Bag<PortValue>& y)
		{
			PortValue pv;
			pv.port = signal;
			pv.value = new object();
			y.insert(pv);
		}
		double ta()
		{
			return sigma;
		}
		void gc_output(adevs::Bag<PortValue>& g)
		{
			adevs::Bag<PortValue>::iterator i;
			for (i = g.begin(); i != g.end(); i++)
				delete (*i).value;
		}
		~genr(){}
		void printState()
		{
			int num_stops_in = num_stop_inputs;
			if (num_stops_in > 0)
				printf("Got %d genr.stop inputs\n",num_stops_in);
		}
		void* save_state()
		{
			state_t* s = new state_t;
			s->active = active;
			s->init_state = init_state;
			s->iterations = iterations;
			s->count = count;
			s->sigma = sigma;
			s->num_stop_inputs = num_stop_inputs;
			return s;
		}
		void restore_state(void* data)
		{
			state_t* s = (state_t*)data;
			active = s->active;
			init_state = s->init_state;
			iterations = s->iterations;
			count = s->count;
			sigma = s->sigma;
			num_stop_inputs = s->num_stop_inputs;
		}
		void gc_state(void* data)
		{
			delete (state_t*)data;
		}

	private:	
		struct state_t
		{
			bool active;
			bool init_state;
			int iterations;
			int count;
			double sigma;
			int num_stop_inputs;
		};

		std::vector<double> pattern;
		bool active;
		bool init_state;
		int iterations;
		int count;
		double sigma;
		int num_stop_inputs;
};

const int genr::stop = 0;
const int genr::start = 1;
const int genr::signal = 2;

#endif
