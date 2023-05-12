#include "pwm.h"
#include <iostream>
#include <list>
using namespace std;
using namespace adevs;

class Transducer:
	public adevs::Atomic<event_t>
{

	private:
		const unsigned N;
		double tL, t, hold, integ;
	public:
		Transducer():
			adevs::Atomic<event_t>(),
			N(20),
			tL(0.0),
			t(0.0),
			hold(0.0),
			integ(0.0)
		{
		}
		double ta()
		{
			return adevs_inf<double>();
		}
		void delta_int(){}
		void delta_ext(double e, const adevs::Bag<event_t>& xb)
		{
			t += e;
			for (auto x: xb)
			{
				if (x.value.open_close.channel == 0)
				{
					integ += hold*(t-tL);
					tL = t;
					hold = x.value.open_close.polarity;
					cout << t << " " << integ << " " << hold << endl;
				}
			}
		}
		void delta_conf(const adevs::Bag<event_t>& xb){}
		void gc_output(adevs::Bag<event_t>&){}
		void output_func(adevs::Bag<event_t>& yb){}
};

class Generator:
	public adevs::Atomic<event_t>
{

	private:
		const double h;
		double t;
	public:
		Generator():
			adevs::Atomic<event_t>(),
			h(1E-3),
			t(0.0)
		{
		}
		double ta() { return h; }
		void delta_int() { t += h; }
		void delta_ext(double e, const adevs::Bag<event_t>& xb){}
		void delta_conf(const adevs::Bag<event_t>& xb){}
		void gc_output(adevs::Bag<event_t>&){}
		void output_func(adevs::Bag<event_t>& yb)
		{
			event_t y;
			y.type = PWM_DUTY_CYCLE;
			y.value.pwm[0] = cos((t+h));
			y.value.pwm[1] = y.value.pwm[2] = 0.0;
			yb.insert(y);
		}
};

int main()
{
	// Create the model
	PWM* pwm = new PWM(1E5);
	Generator* genr = new Generator();
	Transducer* trans = new Transducer();
	SimpleDigraph<event_t>* model = new SimpleDigraph<event_t>();
	model->add(pwm);
	model->add(trans);
	model->add(genr);
	model->couple(genr,pwm);
	model->couple(pwm,trans);
	// Create the simulator
	Simulator<event_t>* sim = new Simulator<event_t>(model);
	while (sim->nextEventTime() <= 2*M_PI)
		sim->execNextEvent();
	delete sim;
	delete model;
	return 0;
}
