#include "adevs.h"
#include "adevs_fmi.h"
#include "adevs_trap.h"
#include "adevs_corrected_euler.h"
#include "TestCircuit.h"
#include <iostream>
using namespace std;
using namespace adevs;

int main()
{
	double cint = 1E-3;
	// Create our model
	TestCircuit* circuit = new TestCircuit();
	// Wrap a set of solvers around it
	Hybrid<double>* hybrid_model =
		new Hybrid<double>
		(
			circuit, // Model to simulate
			new trap<double>(circuit,1E-3,cint,true), // ODE solver
			new discontinuous_event_locator<double>(circuit,1E-8) // Event locator
			// You must use this event locator for OpenModelica because it does
			// not generate continuous zero crossing functions
		);
        // Create the simulator
        Simulator<double>* sim =
			new Simulator<double>(hybrid_model);
		// Run the simulation for ten seconds
		double tL = 0.0;
        while (sim->nextEventTime() < 1.0)
		{
			bool print = sim->nextEventTime() > tL+cint;
			if (print)
			{
				tL = sim->nextEventTime();
				cout << tL;
			}
			sim->execNextEvent();
			if (print)
				cout << "," << circuit->get_circuit_a_i_load() << endl;
		}
		// Cleanup
        delete sim;
		delete hybrid_model;
		// Done!
        return 0;
}
