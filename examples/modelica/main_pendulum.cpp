#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Pendulum.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Pendulum* pendulum = new Pendulum();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		pendulum,
		new rk_45<OMC_ADEVS_IO_TYPE>(pendulum,1E-8,0.01),
		new bisection_event_locator<OMC_ADEVS_IO_TYPE>(pendulum,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, x, y" << endl;
        while (sim->nextEventTime() <= 5.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << pendulum->get_$Px() << " "  
				<< pendulum->get_$Py() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
