#include "DAEexample.h"
#include <iostream>
using namespace adevs;
using namespace std;

Simulator<OMC_ADEVS_IO_TYPE>* sim; 
DAEexample *dae = new DAEexample();

// Test points from OpenModelica test case
// testsuite/simulation/modelica/equations/DAEexample.mo

double test_t[3] = {0.0,0.5,1.0};
double test_x[3] =
{
	0.9,
	0.9814703690055853,
	1.2016745019223243,
};
double test_y[3] =
{
	0.5122399993895521,
	0.6531549234056521,
	1.0259242533530581,
};

void test_point(int index)
{
	// Run the simulation, testing the solution as we go
	while (sim->nextEventTime() <= test_t[index])
	{
		double t = sim->nextEventTime();
		sim->execNextEvent();
		cout << t << " " << dae->get_x() << " " << dae->get_y() << endl;
	}
	cout << dae->get_time() << " " << dae->get_x() << " " << dae->get_y() << endl;
	assert(fabs(dae->get_x()-test_x[index]) < 1E-2);
	assert(fabs(dae->get_y()-test_y[index]) < 1E-2);
}

int main()
{
	Hybrid<OMC_ADEVS_IO_TYPE>* model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		dae,
		new rk_45<OMC_ADEVS_IO_TYPE>(dae,1E-6,0.001),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(dae,1E-6));
	// Create the simulator
	sim = new Simulator<OMC_ADEVS_IO_TYPE>(model);
	for (int i = 0; i < 3; i++)
		test_point(i);
	delete sim;
	delete model;
	return 0;
}
