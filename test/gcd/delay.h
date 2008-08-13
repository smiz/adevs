#ifndef __delay_h_
#define __delay_h_
#include "adevs.h"
#include "object.h"
#include <cstdio>
#include <list>
#include <cassert>

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
		~delay()
		{
		}

	private:	
		double dt, sigma;
		delay_q q;
};

const int delay::in = 0;
const int delay::out = 1;

#endif
