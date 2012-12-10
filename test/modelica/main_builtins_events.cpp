/**
 * Test case for builtin mathematical functions.
 */
#define OMC_ADEVS_IO_TYPE double
#define TOLERANCE 1E-10

#include "adevs.h"
#include "builtins_events.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	builtins_events* test_model = new builtins_events();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(test_model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_model,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 1.0)
		{
			double diff;
			sim->execNextEvent();
			assert(test_model->get_$Py_ceil()==ceil(test_model->get_$Px()));
			assert(test_model->get_$Py_floor()==floor(test_model->get_$Px()));
			assert(fabs(test_model->get_$Py_div()-floor(1.0/test_model->get_$Px()) < 1E-5));
			assert(fabs(test_model->get_$Py_mod()-
						(2.0-floor(2.0/test_model->get_$Px())*test_model->get_$Px()) < 1E-5));
			assert(fabs(test_model->get_$Py_rem()-0.0) < 1E-5);
			assert(test_model->get_$Py_int() == floor(test_model->get_$Px()));
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
