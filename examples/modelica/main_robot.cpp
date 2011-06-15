#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Robot.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Robot* robot = new Robot();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		robot,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(robot,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(robot,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, x, theta" << endl;
        while (sim->nextEventTime() <= 5.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << robot->get_$Px() << " " << robot->get_$Ptheta() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
