/**
 * Test case for builtin mathematical functions.
 */

struct sample_t 
{
	double q1, q2;
};

struct command_t
{
	double T1, T2;
};

union IO_Type
{
	command_t cmd;
	sample_t data;
};

#define OMC_ADEVS_IO_TYPE IO_Type

#include "adevs.h"
#include "Robot.h"
#include "Contro.h"
#include "adevs_modelica_runtime.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

class SampleClock:
	public Atomic<IO_Type>
{
	public:
		SampleClock(double freq):
			Atomic<IO_Type>(),
			freq(freq)
		{
		}
		double ta() { return 1.0/freq; }
		void delta_int(){}
		void delta_ext(double,const Bag<IO_Type>&){}
		void delta_conf(const Bag<IO_Type>&){}
		void gc_output(Bag<IO_Type>&){}
		void output_func(Bag<IO_Type>& yb)
		{
			cerr << "Take sample" << endl;
			IO_Type msg;
			yb.insert(msg);
		}
	private:
		const double freq;
};

class RobotExt:
	public Robot
{
	public:
		RobotExt():
			Robot(),
			doSample(false)
		{
		}
		double time_event_func(const double* q)
		{
			double tSup = Robot::time_event_func(q);
			if (doSample) return 0.0;
			else return tSup;
		}
		void internal_event(double* q, const bool* state_event)
		{
			Robot::internal_event(q,state_event);
			doSample = false;
		}
       void external_event(double* q, double e,
           const adevs::Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
		   Robot::external_event(q,e,xb);
		   doSample = true;
		}
       void confluent_event(double *q, const bool* state_event,
           const adevs::Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
		   Robot::confluent_event(q,state_event,xb);
		   doSample = true;
		}
		void output_func(const double *q, const bool* state_event,
           adevs::Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			Robot::output_func(q,state_event,yb);
			IO_Type d;
			d.data.q1 = get_q1();
			d.data.q2 = get_q2();
			yb.insert(d);
			cerr << "Sample: " << get_q1() << " " << get_q2() << endl;
		}
	private:
		bool doSample;
};

int main()
{
	SampleClock* clk = new SampleClock(100.0);
	RobotExt* arm = new RobotExt();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		arm,
		new rk_45<OMC_ADEVS_IO_TYPE>(arm,1E-6,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(arm,1E-6));
	SimpleDigraph<IO_Type>* model = new SimpleDigraph<IO_Type>();
	model->add(clk);
	model->add(hybrid_model);
	model->couple(clk,hybrid_model);
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(model);
	// Run the simulation, testing the solution as we go
	while (sim->nextEventTime() <= 10.0)
	{
		double t = sim->nextEventTime();
		sim->execNextEvent();
		cout << t << " " << arm->get_x() << " " << arm->get_z() << " "
			<< arm->get_q1() << " " << arm->get_q2() << endl;
	}
	delete sim;
	delete model;
	return 0;
}
