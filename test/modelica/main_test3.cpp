#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "Circuit.h"
#include <iostream>

using namespace std;
using namespace adevs;

class CircuitExt:
	public Circuit
{
	public:
		CircuitExt():
		Circuit(),
		start_time(DBL_MAX)
		{
		}
		void external_event(double* q, double e, const Bag<double>& xb)
		{
			start_time = e;
			set_$PVsrc$PVref(0.0);
		}
		void print_state()
		{
			cout << get_time() << " " << 
				get_$PVsrc$PT$Pv() << " " << 
				endl;
		}
		void test_state()
		{
			double v = 1.0;
			if (get_time() > start_time)
			{
				v = exp(start_time-get_time());
				assert(fabs(v-get_$PVsrc$PT$Pv()) < 1E-6);
			}
			else
			{
				assert(fabs(v-get_$PVsrc$PT$Pv()) < 1E-6);
			}
			assert(fabs(get_$PR2$PT2$Pv()-v/2.0)<1E-6);
			assert(fabs(get_$PR1$PT2$Pv()-v/2.0)<1E-6);
			assert(fabs(get_$PRbridge$PT1$Pi()) < 1E-6);
		}
	private:
		double start_time;
};

int main()
{
	CircuitExt* test_model = new CircuitExt();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_model,
		new rk_45<OMC_ADEVS_IO_TYPE>(test_model,1E-7,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_model,1E-7));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Check initial values
		test_model->print_state();
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 1.0)
		{
			sim->execNextEvent();
			test_model->print_state();
			test_model->test_state();
		}
		Bag<Event<double> > xb;
		Event<double> event(hybrid_model,0.0);
		xb.insert(event);
		sim->computeNextState(xb,1.0);
		while (sim->nextEventTime() <= 5.0)
		{
			sim->execNextEvent();
			test_model->print_state();
			test_model->test_state();
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
