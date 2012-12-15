/**
 * Test case for builtin mathematical functions.
 */
#define OMC_ADEVS_IO_TYPE double
#define TOLERANCE 1E-10

#include "adevs.h"
#include "builtins_events.h"
#include "adevs_modelica_runtime.h"
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
		int n = model->get_$Pn();
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
				model->get_$Py_div1() << " " <<
				model->get_$Py_div2() << " " <<
				model->get_$Py_rem1() << " " <<
				model->get_$Py_rem2() << " " <<
				model->get_$Pn() << " " << 
				endl;

			if (n != model->get_$Pn())
			{
				cerr << t << " Check!" << endl;
				n = model->get_$Pn();
				assert(model->get_$Py_div1() ==
					trunc(model->get_$Py()/model->get_$Px()));
				assert(model->get_$Py_div2() ==
					trunc(model->get_$Px()/model->get_$Py()));
				assert(model->get_$Py_floor() == floor(model->get_$Py()));
				assert(model->get_$Py_ceil() == ceil(model->get_$Py()));
				assert(model->get_$Py_mod1()==model->get_$Py_mod1_compare());
				assert(model->get_$Py_mod2()==model->get_$Py_mod2_compare());
				assert(model->get_$Py_int()==model->get_$Py_floor());
				assert(model->get_$Py_rem1()==
						modelica_rem_real(model->get_$Px(),model->get_$Py()));
				assert(model->get_$Py_rem2()==
						modelica_rem_real(model->get_$Py(),model->get_$Px()));
			}
		}
		delete sim;
		delete hybrid_model;
		return 0;
}
