#include "circuit.h"
#include "pwm.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
  * A test case for the circuit with pwm input.
  */

class CleanCircuit:
	public Circuit
{
	public:
		CleanCircuit():
			Circuit(
				1E-3, // C
				293E-6, // Linv
				100E-6, // Ls
				60.0, // fs
				480.0, // Is
				1.0, // Rs
				480.0, // Vinv
				1E3) // Sample a 1kHz
		{
		}
		double current_comp(double,int,double) { return 0.0; }
};

class Transducer:
	public adevs::Atomic<event_t>
{

	private:
		double t_start, t;
		bool pwm[3];

	public:
		Transducer():
			adevs::Atomic<event_t>(),
			t_start(0.1),
			t(0.0)
		{
			pwm[0] = pwm[1] = pwm[2] = false;
		}
		double ta() { return t_start; }
		void delta_int()
		{
			t += t_start;
			t_start = adevs_inf<double>();
		}
		void delta_ext(double e, const adevs::Bag<event_t>& xb)
		{
			bool pwm_output = false;
			if (t_start < adevs_inf<double>())
				t_start -= e;
			t += e;
			for (auto x: xb)
			{
				if (x.type == ABC_SAMPLE)
					std::cout << "ABC " << t << " " << x.value.data.iabc[0] << " " << x.value.data.iabc[1] << " " << x.value.data.iabc[2] << std::endl;
				else
				{
					pwm_output = true;
					pwm[x.value.open_close.channel] = x.value.open_close.polarity;
				}
			}
			if (pwm_output)
			{
//				std::cout << "PWM " << t << " " << pwm[0] << " " << pwm[1] << " " << pwm[2] << std::endl;
			}
		}
		void delta_conf(const adevs::Bag<event_t>& xb)
		{
			delta_int();
			delta_ext(0.0,xb);
		}
		void gc_output(adevs::Bag<event_t>&){}
		void output_func(adevs::Bag<event_t>& yb)
		{
			event_t y;
			y.type = PWM_DUTY_CYCLE;
			y.value.pwm[0] = 0.25;
			y.value.pwm[1] = 0.5;
			y.value.pwm[2] = 0.75;
			yb.insert(y);
		}
};

int main()
{
	// Create the model
	CleanCircuit* circuit = new CleanCircuit();
	PWM* pwm = new PWM(1E5);
	Transducer* trans = new Transducer();
	SimpleDigraph<event_t>* model = new SimpleDigraph<event_t>();
	model->add(circuit);
	model->add(pwm);
	model->add(trans);
	model->couple(pwm,circuit);
	model->couple(trans,pwm);
	model->couple(pwm,trans);
	model->couple(circuit,trans);
	// Create the simulator
	Simulator<event_t>* sim = new Simulator<event_t>(model);
	while (sim->nextEventTime() <= 10.0)
		sim->execNextEvent();
	delete sim;
	delete model;
	return 0;
}
