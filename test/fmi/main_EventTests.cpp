#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
using namespace std;

int main()
{
	adevs::FMI<int>* fmi =
		new ADEVS_FMI_CONSTRUCTOR("EventTests","{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",2,8,int);
	adevs::corrected_euler<int>* solver1 = new adevs::corrected_euler<int>(fmi,1E-6,0.01);
	adevs::bisection_event_locator<int>* solver2 = new adevs::bisection_event_locator<int>(fmi,1E-7);
	adevs::Hybrid<int>* model =
		new adevs::Hybrid<int>(fmi,solver1,solver2);
	adevs::Simulator<int>* sim = new adevs::Simulator<int>(model);
	while (sim->nextEventTime() < 10.0)
	{
		cout << fmi->get_time() << " " << fmi->get_real(0) << endl;
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	return 0;
}
