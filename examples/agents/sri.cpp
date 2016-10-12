/**
 * This calculates an "agent based" solution to the SRI equations.
 */
#include "adevs.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
using namespace std;
using namespace adevs;

static const int num_agents = 100000;
static const double init_sick = 0.01;
static const double r = 3.0;
static const double d = 5.0;
static const double p = 0.1;
static gsl_rng *rnd = gsl_rng_alloc(gsl_rng_default);
static const int S = 0;
static const int I = 1;
static const int R = 2;
static int pop[3] = { 0, 0, 0 };

class Agent:
	public Atomic<int>
{
	public:
		Agent(int c0):
			Atomic<int>(),
			c(c0),
			td(adevs_inf<double>()),
			tr(adevs_inf<double>())
		{
			if (c == I)
			{
				td = gsl_ran_exponential(rnd,d);
				tr = gsl_ran_exponential(rnd,1.0/r);
			}
			pop[c]++;
		}
		void delta_int()
		{
			assert(c != S);
			if (ta() == td)
			{
				assert(c == I);
				pop[c]--;
				c = R;
				pop[c]++;
				td = tr = adevs_inf<double>();
			}
			else if (ta() < td)
			{
				td -= ta();
				tr = gsl_ran_exponential(rnd,1.0/r);
			}
		}
		void delta_ext(double e, const Bag<int>&)
		{
			if (c == S && gsl_rng_uniform(rnd) < p)
			{
				pop[c]--;
				c = I;
				pop[c]++;
				td = gsl_ran_exponential(rnd,d);
				tr = gsl_ran_exponential(rnd,1.0/r);
			}
			else if (c == I)
			{
				td -= e;
				tr -= e;
			}
		}
		void delta_conf(const Bag<int>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void output_func(Bag<int>& yb)
		{
			if (tr == ta())
				yb.insert(c);
		}
		void gc_output(Bag<int>&){}
		double ta() { return ::min(tr,td); }
	private:
		int c;
		double td, tr;
};

class SIR:
	public Atomic<int>
{
	public:
		SIR():
			Atomic<int>(),
			ss(1.0-init_sick),
			ii(init_sick),
			rr(0.0),
			h(0.01)
		{
		}
		double ta() { return h; }
		void delta_int()
		{
			update(h);
		}
		void delta_ext(double e, const Bag<int>&)
		{
			update(e);
		}
		void delta_conf(const Bag<int>&)
		{
			update(h);
		}
		void output_func(Bag<int>&){}
		void gc_output(Bag<int>&){}
		double get_s() const { return ss; }
		double get_i() const { return ii; }
		double get_r() const { return rr; }
	private:
		double ss, ii, rr;
		const double h;

		void update(double hh)
		{
			double di = p*r*ss*ii-ii/d;
			double ds = -p*r*ss*ii;
			double dr = ii/d;
			ss += hh*ds;
			ii += hh*di;
			rr += hh*dr;
		}
};

class RandomNetwork:
	public Network<int>
{
	public:
		RandomNetwork():
			Network<int>()
		{
			for (int i = 0; i < num_agents; i++)
			{
				if (i < init_sick*num_agents)
					pop.push_back(new Agent(I));
				else
					pop.push_back(new Agent(S));
				pop.back()->setParent(this);
			}
			sir = new SIR();
			sir->setParent(this);
		}
		double get_s() const { return sir->get_s(); }
		double get_i() const { return sir->get_i(); }
		double get_r() const { return sir->get_r(); }
		void getComponents(Set<Devs<int>*>& c)
		{
			for (auto agent: pop)
				c.insert(agent);
			c.insert(sir);
		}
		void route(const int &value, Devs<int>* model, Bag<Event<int> >& r)
		{
			Event<int> xx;
			xx.value = value;
			do
			{
				xx.model = pop[gsl_rng_uniform_int(rnd,pop.size())];
			}
			while (xx.model == model);
			r.insert(xx);
			xx.model = sir;
			r.insert(xx);
		}
		~RandomNetwork()
		{
			for (auto agent: pop)
				delete agent;
		}
		private:
			vector<Agent*> pop;
			SIR* sir;
};

void print(double t, RandomNetwork *world)
{
	cout << t << " " <<
		((double)pop[S]/(double)num_agents) << " " <<
		((double)pop[I]/(double)num_agents) << " " <<
		((double)pop[R]/(double)num_agents) << " " <<
		world->get_s() << " " <<	
		world->get_i() << " " <<	
		world->get_r() << " " << endl;
}

int main()
{
	RandomNetwork *world = new RandomNetwork();
	Simulator<int> *sim = new Simulator<int>(world);
	print(0.0,world);
	while (sim->nextEventTime() < adevs_inf<double>())
	{
		double t = sim->nextEventTime();
		sim->execNextEvent();
		print(t,world);
	}
	delete sim;
	delete world;
}

