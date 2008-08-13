#include "adevs.h"
#include "sampler.h"
#include "ball1d_test_observer.h"
#include "check_ball1d_solution.h"
#include <iostream>
#include <fstream>
using namespace std;
using namespace adevs;

static ofstream soln("state.dat");

/**
Simple test which simulates a bouncing ball. The output value is the height of
the ball. Input values cause the system to produce an output sample immediately.
*/
class bouncing_ball: public rk45_improved<PortValue<double> >
{
	public:
		bouncing_ball():
		rk45_improved<PortValue<double> >(3,0.01,1E-5,1)
		{
			init(0,1.0); // height
			init(1,0.0); // velocity
			init(2,0.0); // time
			sample = false;
			state_change_count = 0;
		}
		void der_func(const double* q, double *dq)
		{
			dq[0] = q[1];
			dq[1] = -2.0;
			dq[2] = 1.0;
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
		void discrete_action(double* q, const Bag<PortValue<double> >& xb,
				const bool* event_flag)
		{
			if (event_flag[0]) 
			{
				q[0] = 0.0;
				q[1] = -q[1];
			}
			sample = xb.size() > 0;
		}
		void discrete_output(const double* q, Bag<PortValue<double> >& yb,
				const bool* event_flag)
		{
			assert(event_flag[0] || event_flag[1]);
			PortValue<double> event(0,q[0]);
			yb.insert(event);
		}
		void state_changed(const double* q)
		{
			state_change_count++;
			soln << q[2] << " " << q[0] << " " << q[1] << " " <<
			check_ball1d_solution(q[2],q[0]) << endl;
			assert(ball1d_soln_ok(q[2],q[0]));
		}
		void gc_output(Bag<PortValue<double> >& g){}
		int getStateChangeCount() const { return state_change_count; }

	private:
		bool sample;
		int state_change_count;
};

int main()
{
	bouncing_ball* ball = new bouncing_ball();
	assert(ball->getNumStateVars() == 3);
	sampler* sample = new sampler(0.01);
	Digraph<double>* model = new Digraph<double>();
	BallObserver* obs = new BallObserver(ball);
	model->add(ball);
	model->add(sample);
	model->couple(sample,0,ball,0);
	model->couple(ball,0,sample,0);
	Simulator<PortValue<double> >* sim = new Simulator<PortValue<double> >(model);
	sim->addEventListener(obs);
	while (sim->nextEventTime() < 10.0)
	{
		sim->execNextEvent();
	}
	assert(ball->getStateChangeCount() > 100); // 10 units of time / 0.1 step size
	assert(obs->getOutputCount() > 1000); // 10 units of time / 0.01 sampling interval
	delete sim;
	delete model;
	delete obs;
	soln.close();
	return 0;
}
