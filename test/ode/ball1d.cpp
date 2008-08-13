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
class bouncing_ball:
	#ifdef RK4
	public rk4<PortValue<double> >
	#endif
	#ifdef RK45
	public rk45<PortValue<double> >
	#endif
{
	public:
		bouncing_ball():
		#ifdef RK4
		rk4<PortValue<double> >(3,0.01)
		#endif 
		#ifdef RK45
		rk45<PortValue<double> >(3,0.01,0.0001,1)
		#endif 
		{
			init(0,1.0); // height
			init(1,0.0); // velocity
			init(2,0.0); // time
			sample = false;
		}
		void der_func(const double* q, double *dq)
		{
			dq[0] = q[1]; // d/dt height = velocity
			dq[1] = -2.0; // d/dt velocity = -2
			dq[2] = 1.0; //  d/dt t = 1
		}
		void state_event_func(const double *q, double* z)
		{
			// Discrete state event function
			if (q[0] <= 0.0 && q[1] < 0.0) z[0] = 1.0; // bounce
			else z[0] = -1.0; // do not bounce
		}
		double time_event_func(const double* q)
		{
			if (sample) return 0.0;
			else return DBL_MAX;
		}
		void discrete_action(double* q, const Bag<PortValue<double> >& xb)
		{
			// change speed if the bounce condition is satisfied
			if (q[0] <= 0.0 && q[1] < 0.0) q[1] = -q[1];
			if (xb.size() > 0) sample = true;
			else sample = false;
		}
		void discrete_output(const double* q, Bag<PortValue<double> >& yb)
		{
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
