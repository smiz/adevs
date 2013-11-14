#include "adevs.h"
#include "Influenza.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	Influenza* influenza = new Influenza();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		influenza,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(influenza,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(influenza,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, sick, infected, not sick, immune" << endl;
        while (sim->nextEventTime() <= 4.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << influenza->get_Sick_Popul_p()
				<< " " << influenza->get_Infected_Popul_p()
				<< " " << influenza->get_Non_Infected_Popul_p()
				<< " " << influenza->get_Immune_Popul_p()
				<< endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
