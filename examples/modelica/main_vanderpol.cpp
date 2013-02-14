#include "adevs.h"
#include "VanDerPol.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	VanDerPol* vanderpol = new VanDerPol();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		vanderpol,
		new rk_45<OMC_ADEVS_IO_TYPE>(vanderpol,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(vanderpol,1E-8));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, x, y" << endl;
        while (sim->nextEventTime() <= 100.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << vanderpol->get_x() << " " << vanderpol->get_y() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
