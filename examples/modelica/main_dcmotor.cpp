#include "adevs.h"
#include "dcmotor.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	dcmotor* dcm = new dcmotor(0,1E-6);
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		dcm,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(dcm,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(dcm,1E-8));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, inductor current, load angle, load speed" << endl;
        while (sim->nextEventTime() <= 10.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << dcm->get_inductor1_i() << " " << dcm->get_load_phi() <<
				" " << dcm->get_load_w() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
