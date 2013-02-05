#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "EventTests.h"
#include <iostream>
using namespace std;
using namespace adevs;

#define ASSERTEQ(x,y) assert(fabs(x-y)<1E-6)

void test_vars(EventTests* model)
{
	cout <<
		model->get_time() << " " <<
		model->get_count() << " " <<
		model->get_x() << " " <<
		model->get_y() << " " <<
		endl;
	assert(fabs(model->get_x()) <= 1.0+2.0*model->getEventEpsilon());
	assert(fabs(model->get_y()) <= 1.0+2.0*model->getEventEpsilon());
	// 2 x accounts for hysteresis
	assert(model->get_count() >= floor(model->get_time()*2.0)+1);
	if (model->get_x()-model->get_x1() > 2.0*model->getEventEpsilon())
	{
		assert(model->get_v1());
		assert(model->get_v2());
		assert(!model->get_v3());
		assert(!model->get_v3());
	}
	// 2 x accounts for hysteresis
	else if (model->get_x()-model->get_x1() < -2.0*model->getEventEpsilon())
	{
		assert(!model->get_v1());
		assert(!model->get_v2());
		assert(model->get_v3());
		assert(model->get_v3());
	}
	// 2 x accounts for hysteresis
	if (model->get_x()-model->get_y() > 2.0*model->getEventEpsilon())
	{
		assert(model->get_w1());
		assert(model->get_w2());
		assert(!model->get_w3());
		assert(!model->get_w3());
	}
	// 2 x accounts for hysteresis
	else if (model->get_x()-model->get_y() < -2.0*model->getEventEpsilon())
	{
		assert(!model->get_w1());
		assert(!model->get_w2());
		assert(model->get_w3());
		assert(model->get_w3());
	}
}

int main()
{
	// Create the circuit
	EventTests* model = new EventTests();
	// Create an atomic model to simulate it
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(model,1E-5,0.01),
		new bisection_event_locator<OMC_ADEVS_IO_TYPE>(model,1E-5));
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
