#include "adevs.h"
#include "adevs_fmi.h"
#include "test1/modelDescription.h"
#include <iostream>
using namespace std;

int main()
{
	test1* fmi = new test1();
	adevs::corrected_euler<int>* solver1 = new adevs::corrected_euler<int>(fmi,1E-6,0.001);
	adevs::bisection_event_locator<int>* solver2 =
		new adevs::bisection_event_locator<int>(fmi,1E-7);
	adevs::Hybrid<int>* model =
		new adevs::Hybrid<int>(fmi,solver1,solver2);
	adevs::Simulator<int>* sim = new adevs::Simulator<int>(model);
	assert(sim->nextEventTime() < 10.0);
	while (sim->nextEventTime() < 10.0)
	{
		double t = fmi->get_time();
		double x = fmi->get_x();
		double a = fmi->get_a();
		double err = fabs(x-exp(a*t));
		assert(err < 1E-3);
		sim->execNextEvent();
	}
	assert(sim->nextEventTime() >= 10.0);
	delete sim;
	delete model;
	return 0;
}
