#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "NonlinearMixed.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	NonlinearMixed* nonlinearmixed = new NonlinearMixed();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		nonlinearmixed,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(nonlinearmixed,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(nonlinearmixed,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		double tL = 0.0;
		cout << "# time, xx, x, y, z" << endl;
        while (sim->nextEventTime() <= 5.0)
		{
			cout << tL << " ";
			cout << nonlinearmixed->get_$Pxx() << " "  
				<< nonlinearmixed->get_$Px() << " "
				<< nonlinearmixed->get_$Py() << " "
				<< nonlinearmixed->get_$Pz() << endl;
			tL = sim->nextEventTime();
			sim->execNextEvent();
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
