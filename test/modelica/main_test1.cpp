#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "test1.h"
#include <iostream>

using namespace std;
using namespace adevs;

class test1Ext:
	public test1
{
	public:
		test1Ext():
		test1(),
		bounce(0),
		resetTime(0.0)
		{
		}
		void internal_event(double* q, const bool* state_event)
		{
			cout << "internal" << endl;
			// Apply internal event function of the super class
			test1::internal_event(q,state_event);
			// Change the direction as needed
			bounce++;
			set_a(-get_a());
			resetTime = get_time();
			update_vars();
			assert((get_a() > 0.0) == get_aAbove());
			assert((get_x() > 1.5) == get_xAbove());
			assert(get_goUp() == (!get_aAbove() && !get_xAbove()));
			assert(get_goDown() == (get_aAbove() && get_xAbove()));
		}
		void print_state()
		{
			cout << get_time() << " " << 
				get_x() << " " << 
				get_DER_x() << " " << 
				get_a() << " " <<
				get_goUp() << " " <<
				get_goDown() << " " <<
				get_aAbove() << " " <<
				get_xAbove() << " " <<
				endl;
		}
		void test_state()
		{
			double x;
			if (bounce % 2 == 0)
				x = 2.0*exp(resetTime-get_time());
			else
				x = exp(get_time()-resetTime);
			assert(fabs(x-get_x()) < 1E-4);
		}
	private:
		int bounce;
		double resetTime;
};

int main()
{
	test1Ext* test_model = new test1Ext();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(test_model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_model,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Check initial values
		assert(test_model->get_time() == 0.0);
		cout << test_model->get_x() << " " <<
			test_model->get_DER_x() << endl;
		assert(fabs(test_model->get_x()-2.0)<1E-6);
		assert(fabs(test_model->get_DER_x()+2.0)<1E-6);
		assert(test_model->get_goUp() == false);
		assert(test_model->get_goDown() == false);
		test_model->print_state();
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 2.0)
		{
			sim->execNextEvent();
			test_model->print_state();
			test_model->test_state();
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
