#include "adevs.h"
#include "adevs_fmi.h"
#include <iostream>
using namespace std;
using namespace adevs;

class oracle:
	public ode_system<double>
{
	public:
		oracle():
			ode_system<double>(3,0),
			test_count(0)
		{
		}
		void init(double* q)
		{
			q[0] = 0.0;
			q[1] = 0.0;
			q[2] = 0.0;
		}
		void der_func(const double *q, double* dq)
		{
			dq[0] = q[1];
			dq[1] = -cos(q[0])*9.82;
			dq[2] = -1.0;
		}
		void state_event_func(const double*,double*){}
		double time_event_func(const double* q) { return q[2]; }
		void internal_event(double* q,const bool*)
		{
			q[2] = 0.01;
		}
		void external_event(double* q, double e, const Bag<double>& xb)
		{
			static const double pi = 3.141592653589793;
			test_count++;
			double test_angle = *(xb.begin());
			double diff = fabs(q[0]-test_angle);
			// Rotate by a full circle?
			if (diff > 1E-4) diff -= 2.0*pi;
			if (!(fabs(diff) < 1E-4))
			{
				cerr << "AGGGH: " << q[0] << "," << test_angle << "," 
					<< diff << endl;
			}
			assert(fabs(diff) < 1E-4);
		}
		void confluent_event(double*,const bool*,const Bag<double>&)
		{
			assert(false);
		}
		void output_func(const double*,const bool*, Bag<double>& yb)
		{
			yb.insert(0);
		}
		void gc_output(Bag<double>&){}
		int getTestCount() { return test_count; }
	private:
		int test_count;
};

class pendulum:
	public FMI<double>
{
	public:
		pendulum():
			FMI<double>(
					"pendulum",
					"{8c4e810f-3df3-4a00-8276-176fa3c9f9e0}",
					2,0,
					"pendulum/binaries/linux64/pendulum.so",
					1E-8),
			query(false)
		{
		}
		double time_event_func(const double*)
		{
			if (query) return 0;
			else return DBL_MAX;
		}
		void internal_event(double* q, const bool* state_events)
		{
			FMI<double>::internal_event(q,state_events);
			query = false;
		}
		void external_event(double* q, double e, const Bag<double>& xb)
		{
			FMI<double>::external_event(q,e,xb);
			query = true;
		}
		void output_func(const double*,const bool*, Bag<double>& yb)
		{
			yb.insert(get_theta());
		}

		double get_theta() { return get_real(7); }
		double get_x() { return get_real(8); }
		double get_y() { return get_real(10); }
		double get_state_set_1() { return get_real(0); }
		double get_state_set_2() { return get_real(1); }
		double get_der_state_set_1() { return get_real(2); }
		double get_der_state_set_2() { return get_real(3); }
		void print_state_matrix()
		{
			cout << get_int(0) << " " << get_int(1) << " " << get_int(2) << " " << get_int(3) << endl;
		}
	private:
		bool query;
};

int main()
{
	// Create the open modelica model
	pendulum* model = new pendulum();
	Hybrid<double>* hybrid_model =
		new Hybrid<double>(
		model,
		new corrected_euler<double>(model,1E-8,0.01),
		new discontinuous_event_locator<double>(model,1E-6));
	// Create the test oracle
	oracle* test_oracle = new oracle();
	Hybrid<double>* hybrid_model_oracle =
		new Hybrid<double>(
		test_oracle,
		new rk_45<double>(test_oracle,1E-8,0.01),
		new linear_event_locator<double>(test_oracle,1E-6));
	// Combine them
	SimpleDigraph<double>* dig_model = new SimpleDigraph<double>();
	dig_model->add(hybrid_model);
	dig_model->add(hybrid_model_oracle);
	dig_model->couple(hybrid_model,hybrid_model_oracle);
	dig_model->couple(hybrid_model_oracle,hybrid_model);
	// Create the simulator
	Simulator<double>* sim =
		new Simulator<double>(dig_model);
	assert(fabs(model->get_theta()) < 1E-6);
	cout << "# time, x, y" << endl;
	while (sim->nextEventTime() <= 25.0)
	{
		cout << sim->nextEventTime() << " ";
		sim->execNextEvent();
		cout << model->get_x() << " "  
		<< model->get_y() << " "
		<< model->get_theta() << " " 
		<< hybrid_model_oracle->getState(0) << " "
	   	<< model->get_state_set_1() << " " 
		<< model->get_state_set_2() << " "
	   	<< model->get_der_state_set_1() << " " 
		<< model->get_der_state_set_2() << " "
		<< endl;
		model->print_state_matrix();
	}
	assert(test_oracle->getTestCount() > 0);
	delete sim;
	delete dig_model;
	return 0;
}
