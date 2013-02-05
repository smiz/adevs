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

void print(double tL, Delay* test_model)
{
	double errx = fabs(1.0-test_model->get_x());
	if (tL > 2.5)
		errx = fabs((sin(tL-2.5)+1.0) - test_model->get_x());
	double errz = fabs(1.0+test_model->get_z());
	if (tL > 0.5)
		errz = fabs((sin(tL-0.5)-1.0) - test_model->get_z());
	cout << tL << " "
		<< test_model->get_z() << " " 
		<< test_model->get_y() << " " 
		<< test_model->get_x() << " " 
		<< errx << " " << errz 
		<< endl;
	assert(errx < 1E-8);
	assert(errz < 1E-8);
}

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
			print(tL,test_model);
			tL = sim->nextEventTime();
			sim->execNextEvent();
		}
		print(tL,test_model);
        delete sim;
		delete hybrid_model;
        return 0;
}
