#include "adevs.h"
#include "Bounce.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Bounce* bounce = new Bounce(0,1E-6);
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		bounce,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(bounce,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(bounce,1E-8));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, x, y" << endl;
        while (sim->nextEventTime() <= 1.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << bounce->get_x() << " " << bounce->get_y() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
