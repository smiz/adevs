#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
using namespace std;
using namespace adevs;

static const double epsilon = 1E-7;
static const double err_tol = 1E-3;

class bounce:
	public FMI<double>
{
	public:
		bounce():
		FMI<double>(
				"bounce",
				"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
				1,1,
				"bounce/binaries/linux64/bounce.so",
				epsilon),
		m_bounce(0),
		m_resetTime(0.0)
		{
		}
		void internal_event(double* q, const bool* state_event)
		{
			cout << "internal" << endl;
			// Apply internal event function of the super class
			FMI<double>::internal_event(q,state_event);
			// Change the direction as needed
			m_bounce++;
			set_a(-get_a());
			m_resetTime = get_time();
			// Reapply internal event function of the super class
			FMI<double>::internal_event(q,state_event);
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
			if (m_bounce % 2 == 0)
				x = 2.0*exp(m_resetTime-get_time());
			else
				x = exp(get_time()-m_resetTime);
			assert(fabs(x-get_x()) < err_tol);
		}
		double get_a() { return get_real(2); }
		double get_x() { return get_real(0); }
		double get_DER_x() { return get_real(1); }
		bool get_goUp() { return get_bool(4); }
		bool get_goDown() { return get_bool(3); }
		bool get_xAbove() { return get_bool(5); }
		bool get_aAbove() { return get_bool(2); }
		void set_a(double a) { set_real(2,a); }
	private:
		int m_bounce;
		double m_resetTime;
};

int main()
{
	bounce* test_model = new bounce();
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
			test_model->get_DER_x() << endl;
		assert(fabs(test_model->get_x()-2.0) < err_tol);
		assert(fabs(test_model->get_DER_x()+2.0)< err_tol);
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
