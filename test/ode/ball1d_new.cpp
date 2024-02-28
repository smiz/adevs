#include "adevs.h"
#include "sampler.h"
#include "check_ball1d_solution.h"
#include <iostream>
#include <typeinfo>
using namespace std;
using namespace adevs;

/**
 * Simple test which simulates a bouncing ball. The output value is the height of
 * the ball. Input values cause the system to produce an output sample immediately.
 */
class bouncing_ball:
	public ode_system<PortValue<double> >
{
	public:
		bouncing_ball():
		ode_system<PortValue<double> >(3,1),
		sample(false),
		phase(FALL)
		{
		}
		void init(double* q)
		{
			last_event_time = 0.0;
			q[0] = 1.0; // Initial height
			q[1] = 0.0; // Initial velocity
			q[2] = 0.0; // Time
		}
		void der_func(const double* q, double* dq)
		{
			dq[0] = q[1];
			dq[1] = -2.0; // For test case
			dq[2] = 1.0;
		}
		void state_event_func(const double* q, double* z)
		{
			if (phase == FALL) z[0] = q[0]; // Bounce if it is going down
			else z[0] = q[1]; // Start falling at apogee
		}
		double time_event_func(const double* q)
		{
			if (sample) return 0.0;
			else return DBL_MAX;
		}
		void internal_event(double* q, const bool* event_flag)
		{
			if (event_flag[0]) 
			{
				if (phase == FALL) // Hit the ground
				{
					phase = CLIMB;
					q[1] = -q[1];
				}
				else // reach apogee
				{
					phase = FALL;
				}
			}
			sample = false;
			last_event_time = q[2];
		}
		void external_event(double* q, double e, const Bag<PortValue<double> >& xb)
		{
			assert(fabs(q[2]-last_event_time-e) < 1E-9); 
			sample = xb.size() > 0;
			last_event_time = q[2];
		}
		void confluent_event(double* q, const bool* event_flag, const Bag<PortValue<double> >& xb)
		{
			internal_event(q,event_flag);
			external_event(q,0.0,xb);
		}
		void output_func(const double* q, const bool* event_flag,
				Bag<PortValue<double> >& yb)
		{
			assert(event_flag[0] || event_flag[1]);
			PortValue<double> event(0,q[0]);
			yb.insert(event);
		}
		void gc_output(Bag<PortValue<double> >& g){}

	private:
		bool sample;
		double last_event_time;
		enum { CLIMB = 0, FALL = 1} phase;
};

class SolutionChecker:
	public EventListener<PortValue<double> >
{
	public:
		SolutionChecker(Hybrid<PortValue<double> >* ball):ball(ball){}
		void stateChange(Atomic<PortValue<double> >* model, double t)
		{
			if (model == ball)
			{
				assert(ball1d_soln_ok(t,ball->getState(0)));
			}
		}
	private:
		Hybrid<PortValue<double> >* ball;
};

void run_test(ode_system<PortValue<double> >* b,
	ode_solver<PortValue<double> > *s,
	event_locator<PortValue<double> >* l)
{
	cerr << "Testing " << typeid(*s).name() << " , " << typeid(*l).name() << endl;
	Hybrid<PortValue<double> >* ball =
		new Hybrid<PortValue<double> >(b,s,l);
	sampler* sample = new sampler(0.01);
	Digraph<double>* model = new Digraph<double>();
	model->add(ball);
	model->add(sample);
	model->couple(sample,0,ball,0);
	model->couple(ball,0,sample,0);
	SolutionChecker* checker = new SolutionChecker(ball);
	Simulator<PortValue<double> >* sim = new Simulator<PortValue<double> >(model);
	sim->addEventListener(checker);
	while (sim->nextEventTime() < 10.0)
	{
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	delete checker;
}

int main()
{
	// Test fast algorithm without interpolation
	bouncing_ball* ball = new bouncing_ball();
	run_test(ball,new corrected_euler<PortValue<double> >(ball,1E-6,0.01),
			new fast_event_locator<PortValue<double> >(ball,1E-7));
	// Test fast algorithm with interpolation
	ball = new bouncing_ball();
	run_test(ball,new corrected_euler<PortValue<double> >(ball,1E-6,0.01),
			new fast_event_locator<PortValue<double> >(ball,1E-7,true));
	// Test linear algorithm
	ball = new bouncing_ball();
	run_test(ball,new corrected_euler<PortValue<double> >(ball,1E-6,0.01),
			new linear_event_locator<PortValue<double> >(ball,1E-7)); 
	// Test RK 45
	ball = new bouncing_ball(); 
	run_test(ball,new rk_45<PortValue<double> >(ball,1E-6,0.01),
			new linear_event_locator<PortValue<double> >(ball,1E-7));
	// Test bisection algorithm 
	ball = new bouncing_ball(); 
	run_test(ball,new corrected_euler<PortValue<double> >(ball,1E-6,0.01),
			new bisection_event_locator<PortValue<double> >(ball,1E-7));
	ball = new bouncing_ball(); 
	run_test(ball,new rk_45<PortValue<double> >(ball,1E-6,0.01),
			new bisection_event_locator<PortValue<double> >(ball,1E-7));
	return 0;
}
