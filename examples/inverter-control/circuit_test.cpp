#include "circuit.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
  * A test case for the circuit component of the
  * inverter control model.
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
				5000.0) // Sample a 1kHz
		{
		}
		double current_comp(double,int,double) { return 0.0; }
};

class Listener:
	public EventListener<event_t>
{
	public:
		Listener():
			EventListener<event_t>()
		{
		}
		void outputEvent(Event<event_t> x, double t)
		{
			cout << t;
			for (int i = 0; i < 3; i++)
			{
				cout << " " << x.value.value.data.iabc[i] << " " << x.value.value.data.vabc[i];
			}
			cout << endl;
		}
		void stateChange(Atomic<event_t>*,double){}
};

int main()
{
	Listener* l = new Listener();
	// Create the model
	CleanCircuit* circuit = new CleanCircuit();
	// Create the simulator
	Simulator<event_t>* sim = new Simulator<event_t>(circuit);
	sim->addEventListener(l);
	while (sim->nextEventTime() <= 10.0)
		sim->execNextEvent();
	delete sim;
	delete circuit;
	delete l;
	return 0;
}
