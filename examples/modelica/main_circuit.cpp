#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Circuit.h"
#include <iostream>
using namespace std;
using namespace adevs;

/**
 * This class extends the Modelica Circuit model to
 * add an output when v = Vref and to adjust Vref
 * on receiving discrete input.
 */
class CircuitExt:
	public Circuit
{
	public:
		CircuitExt():
			Circuit(1), // Add one state event to the base class
			atVref(false) // Flag to indicate when Vref is reached
		{
		}
		/**
		 * Compute the extra state event.
		 */
		void extra_state_event_funcs(double* z)
		{
			if (!atVref)
				z[0] = get_V_T_v()-get_V_Vref();
			else
				z[0] = 1.0;
		}
		/**
		 * Indicate that Vref has been reached when that event
		 * occurs.
		 */
		void internal_event(double* q, const bool* event_flags)
		{
			// Apply the internal event function of the base class
			Circuit::internal_event(q,event_flags);
			// If this is the extra state event, then set atVref to true
			if (event_flags[numStateEvents()]) atVref = true;
		}
		/**
		 * Change Vref on receiving input.
		 */
		void external_event(double* q, double e, const Bag<double>& xb)
		{
			// Apply the external event function of the base class
			Circuit::external_event(q,e,xb);
			// Set the reference voltage and indicate that we are no longer
			// at the reference.
			set_V_Vref(*(xb.begin()));
			atVref = false;
			// Reinitialize the continuous model. This is really only necessary
			// if your discrete event may result in new values for the 
			// state variables (discrete or continuous) of the modelica model.
			update_vars(q,true);
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
			// If this was the reference being reached, then
			// output the current value of the voltage.
			if (event_flags[numStateEvents()])
			{
				yb.insert(get_V_T_v());
				cerr << "Reached Vref=" << get_V_T_v()
					<< " @ t=" << get_time() << endl;
			}
		}
	void print_state()
	{
		cout <<
			get_time() << " " << // Print the time
			get_V_T_v() << " " << // The voltage
			endl;
	}
	private:
		bool atVref;
};

int main()
{
	// Create the circuit
	CircuitExt* model = new CircuitExt();
	// Create an atomic model to simulate it
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(model,1E-5));
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
	model->print_state();
	// Simulate to t = 1
	while (sim->nextEventTime() <= 1.0)
	{
		sim->execNextEvent();
		model->print_state();
	}
	// Inject an input
	Bag<Event<double> > input_bag;
	input_bag.insert(Event<double>(hybrid_model,0.5));
	sim->computeNextState(input_bag,1.0);
	// Simulate from t=1 to t=5
	while (sim->nextEventTime() <= 3.0)
	{
		sim->execNextEvent();
		model->print_state();
	}
	// Done, cleanup
	delete sim;
	delete hybrid_model;
	return 0;
}
