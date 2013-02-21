/**
 * Test case for builtin mathematical functions.
 */
#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Robot.h"
#include "adevs_modelica_runtime.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Robot* model = new Robot();
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
			cout << t << " " << model->get_x() << " " << model->get_z() << " "
				<< model->get_q1() << " " << model->get_q2() << endl;
		}
		delete sim;
		delete hybrid_model;
		return 0;
}
