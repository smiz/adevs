#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Sampler.h"
#include <iostream>
using namespace std;
using namespace adevs;

void test_vars(Sampler* model)
{
	static int n = 0, nn = 0;
	cout <<
		model->get_time() << " " <<
		model->get_$Px() << " " <<
		model->get_$Py() << " " <<
		model->get_$Psample_interval() << " " <<
		endl;
	if (fabs(model->get_time()-n*model->get_$Psample_interval()) <
				model->getEventEpsilon())
	{
		assert(n == nn || model->get_$Px()==model->get_$Py());
		n++;
	}
	else n = nn;
}

int main()
{
	// Create the circuit
	Sampler* model = new Sampler();
	// Create an atomic model to simulate it
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(model,1E-5));
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
	// Simulate 
	while (sim->nextEventTime() <= 5.0)
	{
		sim->execNextEvent();
		test_vars(model);
	}
	// Done, cleanup
	delete sim;
	delete hybrid_model;
	return 0;
}
