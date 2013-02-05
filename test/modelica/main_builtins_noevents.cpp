/**
 * Test case for builtin mathematical functions.
 */
#define OMC_ADEVS_IO_TYPE double
#define TOLERANCE 1E-10

#include "adevs.h"
#include "builtins_noevents.h"
#include <cmath>
#include <iostream>

using namespace std;
using namespace adevs;

int main()
{
	builtins_noevents* test_model = new builtins_noevents();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_model,
		new corrected_euler<OMC_ADEVS_IO_TYPE>(test_model,1E-5,0.01),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_model,1E-5));
        // Create the simulator
        Simulator<OMC_ADEVS_IO_TYPE>* sim =
			new Simulator<OMC_ADEVS_IO_TYPE>(hybrid_model);
		// Run the simulation, testing the solution as we go
        while (sim->nextEventTime() <= 1.0)
		{
			double diff;
			sim->execNextEvent();
			// sqrt
			diff = test_model->get_y_sqrt()-
				sqrt(test_model->get_x());
			cout << "sqrt diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// cos
			diff = test_model->get_y_cos()-
				cos(test_model->get_x());
			cout << "cos diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// sin 
			diff = test_model->get_y_sin()-
				sin(test_model->get_x());
			cout << "sin diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// tan
			diff = test_model->get_y_tan()-
				tan(test_model->get_x());
			cout << "tan diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// exp
			diff = test_model->get_y_exp()-
				exp(test_model->get_x());
			cout << "exp diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// log
			diff = test_model->get_y_log()-
				log(test_model->get_x());
			cout << "log diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// log10
			diff = test_model->get_y_log10()-
				log10(test_model->get_x());
			cout << "log10 diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// asin
			diff = test_model->get_y_asin()-
				asin(test_model->get_x());
			cout << "asin diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// atan
			diff = test_model->get_y_atan()-
				atan(test_model->get_x());
			cout << "atan diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// acos
			diff = test_model->get_y_acos()-
				acos(test_model->get_x());
			cout << "acos diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// atan2
			diff = test_model->get_y_atan2()-
				atan2(1.0,test_model->get_x());
			cout << "atan2 diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// cosh
			diff = test_model->get_y_cosh()-
				cosh(test_model->get_x());
			cout << "cosh diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// sinh
			diff = test_model->get_y_sinh()-
				sinh(test_model->get_x());
			cout << "sinh diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// tanh
			diff = test_model->get_y_tanh()-
				tanh(test_model->get_x());
			cout << "tanh diff " << diff << endl;
			assert(fabs(diff) < TOLERANCE);
			// misc. functions that should not be sensitive to the
			// floating point hardware
			assert(test_model->get_y_abs()==abs(test_model->get_x()));
			assert(test_model->get_y_sign()==1);
		}
        delete sim;
		delete hybrid_model;
        return 0;
}
