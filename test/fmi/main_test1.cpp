#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
using namespace std;

int main()
{
	adevs::FMI<int>* fmi =
		new adevs::FMI<int>(
				"test1",
				"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
				1,0,
				"test1/binaries/linux64/test1.so");
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
		double x = fmi->get_real(0);
		double a = fmi->get_real(2);
		double err = fabs(x-exp(a*t));
		assert(err < 1E-3);
		sim->execNextEvent();
	}
	assert(sim->nextEventTime() >= 10.0);
	delete sim;
	delete model;
	return 0;
}
