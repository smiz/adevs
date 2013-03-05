/**
 * Test case for builtin mathematical functions.
 */

enum Signal { CLK, CMD, INFO };

struct IO_Type
{
	Signal kind;
	double v1, v2;
};

#define OMC_ADEVS_IO_TYPE IO_Type

#include "adevs.h"
#include "Robot.h"
#include "Control.h"
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
			IO_Type msg;
			msg.kind = CLK;
			yb.insert(msg);
		}
	private:
		const double freq;
};

class ControlExt:
	public Control
{
	public:
		ControlExt():
			Control(),
			doCmd(false)
		{
			for (int i = 0; i < 2; i++)
			{
				T[i] = err[i] = ierr[i] = 0.0;
			}
		}
		double time_event_func(const double* q)
		{
			double tSup = Control::time_event_func(q);
			if (doCmd) return 0.0;
			else return tSup;
		}
		void internal_event(double* q, const bool* state_event)
		{
			Control::internal_event(q,state_event);
			doCmd = false;
		}
		void external_event(double* q, double e,
			const adevs::Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			Control::external_event(q,e,xb);
			for (Bag<IO_Type>::const_iterator iter = xb.begin();
				iter != xb.end(); iter++)
			{
				if ((*iter).kind == INFO)
				process_info((*iter).v1,(*iter).v2,e);
			}
		}
		void confluent_event(double *q, const bool* state_event,
			const adevs::Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			double h = time_event_func(q);
			Control::confluent_event(q,state_event,xb);
			for (Bag<IO_Type>::const_iterator iter = xb.begin();
				iter != xb.end(); iter++)
			{
				assert((*iter).kind == INFO);
				process_info((*iter).v1,(*iter).v2,h);
			}
		}
		void output_func(const double *q, const bool* state_event,
           adevs::Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			Control::output_func(q,state_event,yb);
			IO_Type msg;
			msg.kind = CMD;
			msg.v1 = T[0];
			msg.v2 = T[1];
			yb.insert(msg);
		}
	private:
		bool doCmd;
		double err[2], ierr[2], T[2];

		void process_info(double q1, double q2, double h)
		{
			double oldErr[2], derr[2];
			oldErr[0] = err[0];
			oldErr[1] = err[1];
			err[0] = get_qd1()-q1;
			err[1] = get_qd2()-q2;
			for (int i = 0; i < 2; i++)
			{
				derr[i] = (err[i]-oldErr[i])/h;
				ierr[i] += h*err[i];
				T[i] = 2000.0*(err[i]+0.05*derr[i]+ierr[i]/300.0);
				assert(fabs(err[i]) < 0.01);
			}
			// cerr << err[0] << " " << err[1] << endl;
			doCmd = true;
		}
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
			for (Bag<IO_Type>::const_iterator iter = xb.begin();
				iter != xb.end(); iter++)
			{
				if ((*iter).kind == CLK)
					doSample = true;
				else if ((*iter).kind == CMD)
					process_command((*iter).v1,(*iter).v2);
			}
		}
		void confluent_event(double *q, const bool* state_event,
			const adevs::Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			Robot::confluent_event(q,state_event,xb);
			for (Bag<IO_Type>::const_iterator iter = xb.begin();
				iter != xb.end(); iter++)
			{
				if ((*iter).kind == CLK)
					doSample = true;
				else if ((*iter).kind == CMD)
					process_command((*iter).v1,(*iter).v2);
			}
		}
		void output_func(const double *q, const bool* state_event,
           adevs::Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			Robot::output_func(q,state_event,yb);
			IO_Type msg;
			msg.kind = INFO;
			msg.v1 = get_q1();
			msg.v2 = get_q2();
			yb.insert(msg);
		}
	private:

		bool doSample;
		
		void process_command(double T1, double T2)
		{
			set_T(T1,0);
			set_T(T2,1);
			update_vars();
		}
};

int main()
{
	SampleClock* clk = new SampleClock(4000.0);
	RobotExt* arm = new RobotExt();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_arm =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		arm,
		new rk_45<OMC_ADEVS_IO_TYPE>(arm,1E-6,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(arm,1E-6));
	ControlExt* ctrl = new ControlExt();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_ctrl =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		ctrl,
		new rk_45<OMC_ADEVS_IO_TYPE>(ctrl,1E-6,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(ctrl,1E-6));
	SimpleDigraph<IO_Type>* model = new SimpleDigraph<IO_Type>();
	model->add(clk);
	model->add(hybrid_ctrl);
	model->add(hybrid_arm);
	model->couple(clk,hybrid_arm);
	model->couple(hybrid_arm,hybrid_ctrl);
	model->couple(hybrid_ctrl,hybrid_arm);
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(model);
	// Run the simulation, testing the solution as we go
	while (sim->nextEventTime() <= 10.0)
	{
		double t = sim->nextEventTime();
		sim->execNextEvent();
		cout << t << " " << arm->get_x() << " " << arm->get_z() << " "
			<< ctrl->get_xd() << " " << ctrl->get_zd() << " "
			<< arm->get_q1() << " " << arm->get_q2() << " "
			<< ctrl->get_qd1() << " " << ctrl->get_qd2() << endl;
	}
	delete sim;
	delete model;
	return 0;
}
