#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "HelloWorld.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	HelloWorld* hello = new HelloWorld();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		hello,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(hello,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(hello,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, computed soln, actual soln, error" << endl;
        while (sim->nextEventTime() <= 10.0)
		{
			double actual = exp(hello->get_a()*sim->nextEventTime());
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << hello->get_x() << " " << actual << " "
				<< hello->get_x()-actual << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
