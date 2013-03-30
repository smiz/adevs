#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "eventIter.h"
#include <iostream>
#include <cassert>
using namespace std;
using namespace adevs;

void print(eventIter* model)
{
	double cfloor = model->get_c()-floor(model->get_c());
	assert(model->get_floorc() == cfloor);
	assert(cfloor <= 0.5+model->getEventEpsilon());
	assert(cfloor <= 0.25 || model->get_high()==1);
	assert(cfloor >= 0.25 || model->get_high()==0);
	cout << model->get_time() << " "
		<< model->get_c() << " "
		<< model->get_floorc() << " "
		<< model->get_high() <<
		endl;
}

class eventIterExt:
	public eventIter
{
	public:
		eventIterExt():eventIter(){}
		void internal_event(double* q, const bool* state_event)
		{
			eventIter::internal_event(q,state_event);
			update_vars(q,true);
		}
};

int main()
{
	eventIter* test_model = new eventIterExt();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(test_model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_model,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Check initial values
		print(test_model);
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 10.0)
		{
			sim->execNextEvent();
			print(test_model);
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
