#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Linsys.h"
#include <iostream>
using namespace std;
using namespace adevs;

void test_vars(Linsys* model)
{
	double x1 = exp(-0.5*model->get_time());
	double x2 = 2.0*exp(-model->get_time());
	double err1 = fabs(x1-model->get_x(0));
	double err2 = fabs(x2-model->get_x(1));
	cout << model->get_time() << " "
		<< model->get_x(0) << " "
		<< model->get_x(1) << " "
		<< x1 << " " << x2 << " " 
		<< err1 << " " << err2 << endl;
	cout.flush();
	assert(err1 < 1E-7);
	assert(err2 < 1E-7);
}

int main()
{
	// Create the circuit
	Linsys* model = new Linsys();
	// Create an atomic model to simulate it
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		model,
		new rk_45<OMC_ADEVS_IO_TYPE>(model,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(model,1E-5));
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
	// Simulate 
	while (sim->nextEventTime() <= 100.0)
	{
		sim->execNextEvent();
		test_vars(model);
	}
	// Done, cleanup
	delete sim;
	delete hybrid_model;
	return 0;
}
