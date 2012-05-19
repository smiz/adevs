#include "adevs.h"
#include <iostream>

struct sig_t
{
	typedef enum { Left, Right } Motor;
	Motor which;
	double v;
};

#define OMC_ADEVS_IO_TYPE sig_t

#include "Tank.h"
using namespace std;
using namespace adevs;

/**
 * These models produce a motor signal for the tank.
 */
class SignalGenr:
	public Atomic<OMC_ADEVS_IO_TYPE>
{
	public:
		SignalGenr(sig_t::Motor which, double duty_cycle):
			Atomic<OMC_ADEVS_IO_TYPE>(),
			which(which),
			duty_cycle(duty_cycle),
			phase(true)
		{
		}
		double ta()
		{ 
			if ((duty_cycle == 0.0 && !phase) || (duty_cycle == 1.0 && phase))
				return DBL_MAX;
			else if (false)
				return (1.0/7000.0)*(1.0-duty_cycle);
			else
				return duty_cycle*(1.0/7000.0);
		}
		void delta_int() { phase = !phase; }
		void output_func(Bag<OMC_ADEVS_IO_TYPE>& yb)
		{
			sig_t sig;
			sig.which = which;
			sig.v = 7.2*!phase;
			yb.insert(sig);
		}
		void gc_output(Bag<OMC_ADEVS_IO_TYPE>&){}
		void delta_conf(const Bag<OMC_ADEVS_IO_TYPE>&){}
		void delta_ext(double,const Bag<OMC_ADEVS_IO_TYPE>&){}
	private:
		const sig_t::Motor which;
		const double duty_cycle;
		bool phase; // true is on, false is off
};

class SpeedControl:
	public SimpleDigraph<OMC_ADEVS_IO_TYPE>
{
	public:
		SpeedControl(double left_duty, double right_duty):
			SimpleDigraph<OMC_ADEVS_IO_TYPE>()
		{
			SignalGenr* left = new SignalGenr(sig_t::Left,left_duty);
			SignalGenr* right = new SignalGenr(sig_t::Right,right_duty);
			add(left);
			add(right);
			couple(left,this);
			couple(right,this);
		}
};

/**
 * This class extends the Tank with a discrete change to
 * its omega variable when a turn ends.
 */
class ExtendedTank:
	public Tank
{
	public:
		ExtendedTank():
			Tank(0,1E-6)
		{
		}
		void init(double* q)
		{
			Tank::init(q);
			last_turning = get_$Phull$Pturning();	
		}
		void internal_event(double* q, const bool* event_flags)
		{
			Tank::internal_event(q,event_flags);
			if (get_$Phull$Pturning() != last_turning)
			{
				last_turning = get_$Phull$Pturning();
				if (!last_turning) 
				{
					set_$Phull$Pomega(q,0.0);
					update_vars(q);
				}
			}
		}
		void external_event(double* q, double e,
			const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			Bag<OMC_ADEVS_IO_TYPE>::const_iterator iter = xb.begin();
			for (; iter != xb.end(); iter++)
			{
				if ((*iter).which == sig_t::Left)
					set_$Pvin_left((*iter).v);
				else
					set_$Pvin_right((*iter).v);
			}
			update_vars(q);
		}
		void confluent_event(double* q, const bool * event_flags,
			const Bag<OMC_ADEVS_IO_TYPE>& xb)
		{
			internal_event(q,event_flags);
			external_event(q,0.0,xb);
		}
	private:
		int last_turning;
};

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		cout << "Need left and right duty factors." << endl;
		return 0;
	}
	// Get the parameters for the voltage signals
	double left_duty = atof(argv[1]);
	double right_duty = atof(argv[2]);
	// Build the model
	ExtendedTank* tank = new ExtendedTank();
	Hybrid<OMC_ADEVS_IO_TYPE>* tank_solver =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		tank,
		new rk_45<OMC_ADEVS_IO_TYPE>(tank,1E-2,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(tank,1E-8));
	SpeedControl* speedControl = new SpeedControl(left_duty,right_duty);
	SimpleDigraph<OMC_ADEVS_IO_TYPE>* model =
		new SimpleDigraph<OMC_ADEVS_IO_TYPE>();
	model->add(tank_solver);
	model->add(speedControl);
	model->couple(speedControl,tank_solver);
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(model);
	// Run the simulation
	cout << "# time, v, omega, theta, x, y" << endl;
	while (sim->nextEventTime() <= 1.0)
	{
		double t = sim->nextEventTime();
		sim->execNextEvent();
		cout << t << " "
			<< tank->get_$Phull$Pv() << " "
	    	<< tank->get_$Phull$Pomega() << " "
	    	<< tank->get_$Phull$Ptheta() << " "
	    	<< tank->get_$Phull$Px() << " "
	    	<< tank->get_$Phull$Py() << " "
	    	<< endl;
	}
	delete sim;
	delete model;
	return 0;
}
