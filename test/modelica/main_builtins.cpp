/**
 * Test case for builtin mathematical functions.
 */
#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "builtins.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	builtins* test_model = new builtins();
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
			sim->execNextEvent();
			assert(test_model->get_$Py_sqrt()==sqrt(test_model->get_$Px()));
			assert(test_model->get_$Py_cos()==cos(test_model->get_$Px()));
			assert(test_model->get_$Py_sin()==sin(test_model->get_$Px()));
			assert(test_model->get_$Py_tan()==tan(test_model->get_$Px()));
			assert(test_model->get_$Py_abs()==abs(test_model->get_$Px()));
			assert(test_model->get_$Py_ceil()==ceil(test_model->get_$Px()));
			assert(test_model->get_$Py_floor()==floor(test_model->get_$Px()));
			assert(test_model->get_$Py_log()==log(test_model->get_$Px()));
			assert(test_model->get_$Py_log10()==log10(test_model->get_$Px()));
			assert(test_model->get_$Py_exp()==exp(test_model->get_$Px()));
			assert(test_model->get_$Py_sign()==1);
			assert(fabs(test_model->get_$Py_div()-floor(1.0/test_model->get_$Px()) < 1E-5));
			assert(fabs(test_model->get_$Py_mod()-
						(2.0-floor(2.0/test_model->get_$Px())*test_model->get_$Px()) < 1E-5));
			assert(fabs(test_model->get_$Py_rem()-0.0) < 1E-5);
			assert(test_model->get_$Py_int() == floor(test_model->get_$Px()));
			assert(test_model->get_$Py_asin()==asin(test_model->get_$Px()));
			assert(test_model->get_$Py_atan()==atan(test_model->get_$Px()));
			assert(test_model->get_$Py_acos()==acos(test_model->get_$Px()));
			assert(test_model->get_$Py_atan2()==atan2(1.0,test_model->get_$Px()));
			assert(test_model->get_$Py_cosh()==cosh(test_model->get_$Px()));
			assert(test_model->get_$Py_sinh()==sinh(test_model->get_$Px()));
			assert(test_model->get_$Py_tanh()==tanh(test_model->get_$Px()));
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
