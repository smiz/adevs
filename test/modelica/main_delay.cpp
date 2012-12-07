/**
 * Test case for builtin mathematical functions.
 */
#define OMC_ADEVS_IO_TYPE double
#define TOLERANCE 1E-10

#include "adevs.h"
#include "Delay.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Delay* test_model = new Delay();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_model,
		new rk_45<OMC_ADEVS_IO_TYPE>(test_model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_model,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Run the simulation, testing the solution as we go
		double tL = 0.0;
        while (sim->nextEventTime() <= 10.0)
		{
			double errx = fabs(1.0-test_model->get_$Px());
			if (tL > 2.5)
				errx = fabs((sin(tL-2.5)+1.0) - test_model->get_$Px());
			double errz = fabs(1.0-test_model->get_$Pz());
			if (tL > 0.5)
				errz = fabs((sin(tL-0.5)-1.0) - test_model->get_$Pz());
			cout << tL << " "
				<< test_model->get_$Py() << " " 
				<< test_model->get_$Px() << " " 
				<< test_model->get_$Pz() << " " 
				<< errx << " " << errz 
				<< endl; 
			tL = sim->nextEventTime();
			sim->execNextEvent();
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
