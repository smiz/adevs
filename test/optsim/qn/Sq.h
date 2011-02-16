#ifndef Sq_h_
#define Sq_h_
#include "adevs.h"
#include <list>
#include <iostream>

class Sq: public adevs::Atomic<int>
{
	public:
		Sq():adevs::Atomic<int>(),r(seed++) { ttg = r.exponential(1.0)+1.0; }
		void delta_int() { q.pop_front(); ttg = r.exponential(1.0)+1.0; }
		void delta_ext(double e, const adevs::Bag<int>& xb)
		{
			if (!q.empty()) ttg -= e;
			for (adevs::Bag<int>::const_iterator i = xb.begin();
					i != xb.end(); i++)
				q.push_back(*i);
		}
		void delta_conf(const adevs::Bag<int>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(adevs::Bag<int>& yb) { yb.insert(q.front()); }
		double ta() { if (q.empty()) return DBL_MAX; else return ttg; }
		void gc_output(adevs::Bag<int>&){}
		double lookahead() { return 1.0; }
		void beginLookahead()
		{
			chkpt.r = r;
			chkpt.q = q;
			chkpt.ttg = ttg;
		}
		void endLookahead()
		{
			r = chkpt.r;
			q = chkpt.q;
			ttg = chkpt.ttg;
		}
	private:
		adevs::rv r;
		std::list<int> q;
		double ttg;
		struct checkpoint_t
		{
			adevs::rv r;
			std::list<int> q;
			double ttg;
		};
		checkpoint_t chkpt;
		static int seed;
};

class Ql: public adevs::Network<int>
{
	public:
		Ql(int size):
			adevs::Network<int>(),size(size),sq(new Sq[size])
		{
			for (int i = 0; i < size; i++) sq[i].setParent(this);
		}
		void getComponents(adevs::Set<adevs::Devs<int>*>& c)
		{
			for (int i = 0; i < size; i++) c.insert(&(sq[i]));
		}
		void route(const int &value, adevs::Devs<int> *model,
				adevs::Bag<adevs::Event<int> > &r) {
			// Input to the line of queues
			if (model == this) r.insert(adevs::Event<int>(&(sq[0]),value));
			// Output from the line of queues
			else if (model == &(sq[size-1]))
				r.insert(adevs::Event<int>(this,value));
			// Advance in the line
			else for (int i = 0; i < size; i++) {
				if (model == &(sq[i])) {
					r.insert(adevs::Event<int>(&(sq[i+1]),value));
					return;
				}
			}
		}
		double lookahead() { return size*(sq[0].lookahead()); }
		~Ql() { delete [] sq; }
	private:
		const int size;
		Sq* sq;
};

#endif
