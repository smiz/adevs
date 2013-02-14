#include "adevs.h"
#include "BouncingBall.h"
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	BouncingBall* ball = new BouncingBall(0,1E-6);
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		ball,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(ball,1E-8,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(ball,1E-8));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		cout << "# time, h, v, flying" << endl;
        while (sim->nextEventTime() <= 3.0)
		{
			cout << sim->nextEventTime() << " ";
			sim->execNextEvent();
			cout << ball->get_h() << " " <<
				ball->get_v() << " " <<
				ball->get_flying() << endl;
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
