#ifndef __delay_h_
#define __delay_h_
#include "adevs.h"
#include "object.h"
#include <cstdio>
#include <list>
#include <cassert>
#include <iostream>

typedef std::list<std::pair<double,object*> > delay_q;

class delay: public adevs::Atomic<PortValue>
{
	public:

		static const int in;
		static const int out;

		delay (double t):
		adevs::Atomic<PortValue>(),
		dt(t),
		sigma(DBL_MAX)
		{
		}
		void delta_int()
		{
			delay_q::iterator i;
			for (i = q.begin(); i != q.end(); i++)
			{
				(*i).first -= ta();
			}
			while (!q.empty())
			{
				if (q.front().first <= 1E-14)
				{
					delete q.front().second;
					q.pop_front();
				}
				else
				{
					assert(q.front().first >= 0.0);
					sigma = q.front().first;
					return;
				}
			}
			sigma = DBL_MAX;
		}
		void delta_ext(double e, const adevs::Bag<PortValue>& x)
		{
			delay_q::iterator i;
			for (i = q.begin(); i != q.end(); i++)
			{
				(*i).first -= e;
			}
			adevs::Bag<PortValue>::const_iterator xi;
			for (xi = x.begin(); xi != x.end(); xi++)
			{
				assert((*xi).port == in);
				q.push_back(
					std::pair<double,object*>(dt,new object(*((*xi).value)))
				);
			}
			assert(q.front().first >= 0.0);
			sigma = q.front().first;
		}
		void delta_conf(const adevs::Bag<PortValue>& x)
		{
			delta_int();
			delta_ext(0.0,x);
		}
		void output_func (adevs::Bag<PortValue>& y)
		{
			delay_q::iterator i;
			for (i = q.begin(); i != q.end(); i++)
			{
				if ((*i).first <= ta())
				{
					PortValue pv;
					pv.port = out;
					pv.value = new object(*((*i).second));
					y.insert(pv);
				}
			}
		}
		double ta()
		{
			return sigma;
		}
		void gc_output(adevs::Bag<PortValue>& g)
		{
			adevs::Bag<PortValue>::const_iterator i;
			for (i = g.begin(); i != g.end(); i++)
			{
				delete (*i).value;
			}
		}
		void* save_state()
		{
			state_t* s = new state_t;
			s->dt = dt;
			s->sigma = sigma;
			delay_q::iterator i;
			for (i = q.begin(); i != q.end(); i++)
			{
				s->q.push_back(*i);
				object* obj = (*i).second;
				s->q.back().second = new object(*obj);
				assert(obj != s->q.back().second);
			}
			assert(s->q.size() == q.size());
			return s;
		}
		void restore_state(void *data)
		{
			state_t* s = (state_t*)data;
			dt = s->dt;
			sigma = s->sigma;
			while (!q.empty())
			{
				delete q.front().second;
				q.pop_front();
			}
			delay_q::iterator i;
			for (i = s->q.begin(); i != s->q.end(); i++)
			{
				q.push_back(*i);
				object* obj = (*i).second;
				q.back().second = new object(*obj);
			}
			assert(s->q.size() == q.size());
		}
		void gc_state(void* data)
		{
			state_t* s = (state_t*)data;
			while (!s->q.empty())
			{
				delete s->q.front().second;
				s->q.pop_front();
			}
			delete s;
		}
		~delay()
		{
		}

	private:
		struct state_t
		{
			double dt, sigma;
			delay_q q;
		};
		double dt, sigma;
		delay_q q;
};

const int delay::in = 0;
const int delay::out = 1;

#endif
