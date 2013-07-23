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

static double modelica_rem_real(double x, double y)
{
	return x-(trunc(x/y)*y);
}

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
		int n = model->get_n();
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 10.0)
		{
			double t = sim->nextEventTime();
			sim->execNextEvent();
			cout << t << " " <<
				model->get_x() << " " <<
				model->get_y() << " " <<
				model->get_y_floor() << " " <<
				model->get_y_int() << " " <<
				model->get_y_ceil() << " " <<
				model->get_y_mod1() << " " <<
				model->get_y_mod1_compare() << " " <<
				model->get_y_mod2() << " " <<
				model->get_y_mod2_compare() << " " <<
				model->get_y_div1() << " " <<
				model->get_y_div2() << " " <<
				model->get_y_rem1() << " " <<
				model->get_y_rem2() << " " <<
				model->get_n() << " " << 
				endl;

			if (n != model->get_n())
			{
				cerr << t << " Check!" << endl;
				n = model->get_n();
				assert(model->get_y_div1() ==
					trunc(model->get_y()/model->get_x()));
				assert(model->get_y_div2() ==
					trunc(model->get_x()/model->get_y()));
				assert(model->get_y_floor() == floor(model->get_y()));
				assert(model->get_y_ceil() == ceil(model->get_y()));
				assert(model->get_y_mod1()==model->get_y_mod1_compare());
				assert(model->get_y_mod2()==model->get_y_mod2_compare());
				assert(model->get_y_int()==model->get_y_floor());
				assert(model->get_y_rem1()==
						modelica_rem_real(model->get_x(),model->get_y()));
				assert(model->get_y_rem2()==
						modelica_rem_real(model->get_y(),model->get_x()));
			}
		}
		delete sim;
		delete hybrid_model;
		return 0;
}
