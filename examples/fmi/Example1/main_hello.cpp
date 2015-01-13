#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
 * This model will sample its own continuous state and generate an output
 * with a frequency f in Hertz.
 */
class HelloWorld:
	// Derive model from the adevs FMI class
	public FMI<double>
{
	public:
		// Constructor loads the FMI and sets the output frequency
		HelloWorld():
			// Call FMI constructor
			FMI<double>
			(
			 	"HelloWorld", // model name from modelDescription.xml
		 		"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}", // GUID from modelDescription.xml
				1, // Number of derivative variables
				0, // numberOfEventIndicators from modelDescription.xml
				"binaries/linux64/HelloWorld.so" // Location of the shared object file produced by omc
			),
			// Set frequency of output to 10 Hz
			f(10.0),
			// Time of first output
			tnext(1.0/f)
		{
		}
		// Internal transition function
		void internal_event(double* q, const bool* state_event)
		{
			assert(get_time() == q[1]); // These should match
			// Update the time to the next event
			tnext += 1.0/f;
		}
		// Time remaining to the next sample
		double time_event_func(const double* q)
		{
			assert(get_time() == q[1]); // These should match
			return tnext - get_time();
		}
		// Print state at each output event
		void output_func(const double* q, const bool* state_event, Bag<double>& yb)
		{
			// Get the model state. This is real variable 0 according to modelDescription.xml
			double x = get_real(0);
			// Get the model parameter. This is real variable 2 according to modelDescription.xml
			double a = get_real(2);
			assert(q[0] == x); // These should match
			// Output our state
			yb.insert(x);
			// Print the time, computed state, and exact state
			cout << get_time() << " " << x << " " << exp(a*get_time()) << endl;
		}
	private:
		const double f;
		double tnext;
};

int main()
{
	// Create our model
	FMI<double>* hello = new HelloWorld();
	// Wrap a set of solvers around it
	Hybrid<double>* hybrid_model =
		new Hybrid<double>
		(
			hello, // Model to simulate
			new corrected_euler<double>(hello,1E-5,0.01), // ODE solver
			new discontinuous_event_locator<double>(hello,1E-5) // Event locator
			// You must use this event locator for OpenModelica because it does
			// not generate continuous zero crossing functions
		);
        // Create the simulator
        Simulator<double>* sim =
			new Simulator<double>(hybrid_model);
		// Run the simulation for ten seconds
        while (sim->nextEventTime() <= 10.0)
			sim->execNextEvent();
		// Cleanup
        delete sim;
		delete hybrid_model;
		// Done!
        return 0;
}
