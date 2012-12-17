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
	builtins_events* model = new builtins_events();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(model,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(model,1E-8));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 10.0)
		{
			double t = sim->nextEventTime();
			sim->execNextEvent();
			cout << t << " " <<
				model->get_$Px() << " " <<
				model->get_$Py() << " " <<
				model->get_$Py_floor() << " " <<
				model->get_$Py_int() << " " <<
				model->get_$Py_ceil() << " " <<
				model->get_$Py_mod1() << " " <<
				model->get_$Py_mod1_compare() << " " <<
				model->get_$Py_mod2() << " " <<
				model->get_$Py_mod2_compare() << " " <<
				model->get_$Py_div() << " " <<
				model->get_$Py_div_expr() << 
				endl;
			assert(model->get_$Py_floor() <= model->get_$Py()+
					model->getEventEpsilon());
			assert(fabs(model->get_$Py_floor()-model->get_$Py())
					<= 1.0+model->getEventEpsilon());
			assert(model->get_$Py_ceil() >= model->get_$Py()-
					model->getEventEpsilon());
			assert(fabs(model->get_$Py_ceil()-model->get_$Py())
					<= 1.0+model->getEventEpsilon());
			assert(model->get_$Py_mod1()==model->get_$Py_mod1_compare());
			assert(model->get_$Py_mod2()==model->get_$Py_mod2_compare());
			assert(model->get_$Py_int()==model->get_$Py_floor());
			assert(fabs(model->get_$Py_div()-trunc(model->get_$Py_div_expr()))
					<= 1.0+model->getEventEpsilon());
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
