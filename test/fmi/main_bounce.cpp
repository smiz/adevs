#include "adevs.h"
#include "adevs_fmi.h"
#include "bounce/modelDescription.h"
#include <iostream>
using namespace std;
using namespace adevs;

static const double epsilon = 1E-7;
static const double err_tol = 1E-3;

class bounce2:
	public bounce
{
	public:
		bounce2():
			bounce(),
			m_bounce(0),
			m_resetTime(0.0)
		{
		}
		void internal_event(double* q, const bool* state_event)
		{
			cout << "internal" << endl;
			// Apply internal event function of the super class
			bounce::internal_event(q,state_event);
			// Change the direction as needed
			m_bounce++;
			set_a(-get_a());
			m_resetTime = get_time();
			// Reapply internal event function of the super class
			bounce::internal_event(q,state_event);
			assert((get_a() > 0.0) == get_aAbove());
			assert((get_x() > 1.5) == get_xAbove());
			assert(get_goUp() == (!get_aAbove() && !get_xAbove()));
			assert(get_goDown() == (get_aAbove() && get_xAbove()));
		}
		void print_state()
		{
			cout << get_time() << " " << 
				get_x() << " " << 
				get_der_x_() << " " << 
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
			if (m_bounce % 2 == 0)
				x = 2.0*exp(m_resetTime-get_time());
			else
				x = exp(get_time()-m_resetTime);
			assert(fabs(x-get_x()) < err_tol);
		}
	private:
		int m_bounce;
		double m_resetTime;
};

int main()
{
	bounce2* test_model = new bounce2();
	Hybrid<double>* hybrid_model =
		new Hybrid<double>(
		test_model,
		new corrected_euler<double>(test_model,epsilon,0.001),
		new discontinuous_event_locator<double>(test_model,epsilon));
        // Create the simulator
        Simulator<double>* sim =
			new Simulator<double>(hybrid_model);
		// Check initial values
		assert(test_model->get_time() == 0.0);
		cout << test_model->get_x() << " " <<
			test_model->get_der_x_() << endl;
		assert(fabs(test_model->get_x()-2.0) < err_tol);
		assert(fabs(test_model->get_der_x_()+2.0)< err_tol);
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
