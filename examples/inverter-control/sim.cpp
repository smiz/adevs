#include "harmonic_compensator.h"
#include "pwm.h"
#include <iostream>
using namespace std;
using namespace adevs;

class Listener:
	public EventListener<event_t>
{
	public:
		Listener():EventListener<event_t>(){}
		void outputEvent(Event<event_t> x, double t)
		{
			if (x.value.type == ABC_SAMPLE)
			{
				cout << t;
				for (int i = 0; i < 3; i++)
					cout << " " << x.value.value.data.iabc[i];
				cout << endl;
			}
		}
		void stateChange(Atomic<event_t>*,double){}
};

static const double C = 0.00114047; //1E-3;
static const double Linv = 0.000279834; // 293E-6;
static const double G = 0.562996; //0.5;
static const double H = 0.100675; // 0.1;

int main()
{
	Listener* l = new Listener();
	// Create the model
	HarmonicCircuit* circuit = new HarmonicCircuit(C,Linv);
	HarmonicCompensator* control = new HarmonicCompensator(G,H);
	PWM* pwm = new PWM(33000.0);
	SimpleDigraph<event_t>* model = new SimpleDigraph<event_t>();
	model->add(circuit);
	model->add(control);
	model->add(pwm);
	model->couple(circuit,control);
	model->couple(control,pwm);
	model->couple(pwm,circuit);
	// Create the simulator
	Simulator<event_t>* sim = new Simulator<event_t>(model);
	sim->addEventListener(l);
	while (sim->nextEventTime() <= 10.0)
		sim->execNextEvent();
	delete sim;
	delete circuit;
	delete l;
	return 0;
}
