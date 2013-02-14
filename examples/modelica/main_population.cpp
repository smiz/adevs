#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Population.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Population* pop = new Population();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		pop,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(pop,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(pop,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, computed soln, actual soln, error" << endl;
		cout << 0 << " " << pop->get_ps() << " " << pop->get_pa() << endl;
        while (sim->nextEventTime() <= 2.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << pop->get_ps() << " " 
				<< pop->get_pa() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
