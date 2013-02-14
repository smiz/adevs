#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Robot.h"
#include <iostream>

using namespace std;
using namespace adevs;

#ifndef PI
#define PI 3.1415926535897931
#endif

/**
 * This class extends the Modelica Robot with a discrete angle
 * sensor.
 */
class RobotWithSensor:
	public Robot
{
	public:
		RobotWithSensor():
			Robot(2),
			mAngle(2.0*PI/1024.0)
		{
		}
		void init(double* q)
		{
			Robot::init(q);
			k = int(get_theta()/mAngle);
		}
		void extra_state_event_funcs(double* z)
		{
			z[0] = get_theta()-mAngle*(k-1);
			z[1] = get_theta()-mAngle*(k+1);
		}
		void internal_event(double* q, const bool* event_flags)
		{
			Robot::internal_event(q,event_flags);
			if (event_flags[numStateEvents()]) k--;
			else if (event_flags[numStateEvents()+1]) k++;
		}
		void external_event(double* q, double e, const Bag<double>& xb)
		{
			Robot::external_event(q,e,xb);
			set_Fcontrol(*(xb.begin()));
			update_vars(q);
		}
		void confluent_event(double* q, const bool * event_flags,
			const Bag<double>& xb)
		{
			internal_event(q,event_flags);
			external_event(q,0.0,xb);
		}
		void output_func(const double* q, const bool* event_flags,
			Bag<double>& yb)
		{
			if (event_flags[numStateEvents()])
				yb.insert((k-1)*mAngle);
			else if (event_flags[numStateEvents()+1])
				yb.insert((k+1)*mAngle);
		}
		double getSensorAngle() const { return mAngle*k; }
	private:
		const double mAngle;
		int k;
};

class PIDControl:
	public Atomic<double>
{
	public:
		PIDControl():Atomic<double>(),
			err(0.0),err_int(0.0),
			csignal(0.0),send_control(false){}
		void delta_int() { send_control = false; }
		void delta_ext(double e, const Bag<double>& xb)
		{
			// Error is the difference of the arm angle and zero
			double new_err = -(*(xb.begin())); // New error value
			err_int += new_err*e; // Integral of the error
			double derr = (new_err-err)/e; // Derivative of the error
			err = new_err; // Value of the error
			csignal = 50.0*err+5.0*err_int+1.0*derr; // Control signal
			send_control = true; // Send a new control value
		}
		void delta_conf(const Bag<double>& xb)
		{
			delta_int(); delta_ext(0.0,xb);
		}
		double ta() { if (send_control) return 0.0; return DBL_MAX; }
		void output_func(Bag<double>& yb) { yb.insert(csignal); }
		void gc_output(Bag<double>&){}
	private:
		double err, err_int, csignal;
		bool send_control;
};

static RobotWithSensor* robot;

Devs<double>* makeRobotOnly()
{
	robot = new RobotWithSensor();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		robot,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(robot,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(robot,1E-5));
	return hybrid_model;
}

Devs<double>* makeRobotWithControl()
{
	Devs<double>* robot_solver = makeRobotOnly();
	PIDControl* control = new PIDControl();
	SimpleDigraph<double>* model = new SimpleDigraph<double>();
	model->add(robot_solver);
	model->add(control);
	model->couple(robot_solver,control);
	model->couple(control,robot_solver);
	return model;
}

int main()
{
	Devs<double>* model = makeRobotWithControl();
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(model);
	cout << "# time, x, theta, sensed theta" << endl;
	while (sim->nextEventTime() <= 5.0)
	{
		cout << sim->nextEventTime() << " ";
		sim->execNextEvent();
		cout << robot->get_x() << " " << robot->get_theta() << 
			" " << robot->getSensorAngle() << endl;
	}
	delete sim;
	delete model;
	return 0;
}
