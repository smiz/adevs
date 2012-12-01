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
		model->get_$Pcount() << " " <<
		model->get_$Pv1() << " " <<
		model->get_$Px1() << " " <<
		model->get_$Px() << " " <<
		endl;
	// 2 x accounts for hysteresis
	assert(model->get_$Pcount() >= floor(model->get_time()*2.0)+1);
	if (model->get_$Px()-model->get_$Px1() > 2.0*model->getEventEpsilon())
	{
		assert(model->get_$Pv1());
		assert(model->get_$Pv2());
		assert(!model->get_$Pv3());
		assert(!model->get_$Pv3());
	}
	// 2 x accounts for hysteresis
	else if (model->get_$Px()-model->get_$Px1() < -2.0*model->getEventEpsilon())
	{
		assert(!model->get_$Pv1());
		assert(!model->get_$Pv2());
		assert(model->get_$Pv3());
		assert(model->get_$Pv3());
	}
	// 2 x accounts for hysteresis
	if (model->get_$Px()-model->get_$Py() > 2.0*model->getEventEpsilon())
	{
		assert(model->get_$Pw1());
		assert(model->get_$Pw2());
		assert(!model->get_$Pw3());
		assert(!model->get_$Pw3());
	}
	// 2 x accounts for hysteresis
	else if (model->get_$Px()-model->get_$Py() < -2.0*model->getEventEpsilon())
	{
		assert(!model->get_$Pw1());
		assert(!model->get_$Pw2());
		assert(model->get_$Pw3());
		assert(model->get_$Pw3());
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
