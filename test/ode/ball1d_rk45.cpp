#include "adevs.h"
#include "adevs_rk45.h"
#include "sampler.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
Simple test which simulates a bouncing ball. The output value is the height of
the ball. Input values cause the system to produce an output sample immediately.
*/
class bouncing_ball: public rk45<PortValue<double> >
{
	public:
		bouncing_ball(double h0):
		rk45<PortValue<double> >(2,0.01,1E-5,1)
		{
			init(0,h0);
			init(1,0.0);
			sample = false;
		}
		void der_func(const double* q, double *dq)
		{
			dq[0] = q[1];
			dq[1] = -9.8;
		}
		void state_event_func(const double *q, double* z)
		{
			z[0] = q[0];
		}
		double time_event_func(const double* q)
		{
			if (sample) return 0.0;
			else return DBL_MAX;
		}
		void discrete_action(double* q, const Bag<PortValue<double> >& xb)
		{
			if (q[0] <= 0.0) q[1] = -q[1];
			if (xb.size() > 0) sample = true;
			else sample = false;
		}
		void discrete_output(const double* q, Bag<PortValue<double> >& yb)
		{
			PortValue<double> event(0,q[0]);
			yb.insert(event);
		}
		void gc_output(Bag<PortValue<double> >& g){}

	private:
		bool sample;
};

int main()
{
	bouncing_ball* ball = new bouncing_ball(1.0);
	sampler* sample = new sampler(0.01);
	Digraph<double>* model = new Digraph<double>();
	model->add(ball);
	model->add(sample);
	model->couple(sample,0,ball,0);
	model->couple(ball,0,sample,0);
	Simulator<PortValue<double> >* sim = new Simulator<PortValue<double> >(model);
	while (sim->nextEventTime() < 10.0)
	{
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	return 0;
}
