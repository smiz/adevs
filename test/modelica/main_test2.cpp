#define OMC_ADEVS_IO_TYPE double

#include "adevs.h"
#include "test2.h"
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
			test_count++;
			double test_angle = *(xb.begin());
			assert(fabs(q[0]-test_angle) < 1E-4);
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

class test2Ext:
	public test2
{
	public:
		test2Ext():test2(),query(false){}
		double time_event_func(const double*)
		{
			if (query) return 0;
			else return DBL_MAX;
		}
		void internal_event(double* q, const bool* state_events)
		{
			test2::internal_event(q,state_events);
			query = false;
		}
		void external_event(double*,double,const Bag<double>&)
		{
			query = true;
		}
		void output_func(const double*,const bool*, Bag<double>& yb)
		{
			yb.insert(get_$Ptheta());
		}
	private:
		bool query;
};

int main()
{
	// Create the open modelica model
	test2Ext* pendulum = new test2Ext();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		pendulum,
		new rk_45<OMC_ADEVS_IO_TYPE>(pendulum,1E-6,0.001),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(pendulum,1E-6));
	// Create the test oracle
	oracle* test_oracle = new oracle();
	Hybrid<OMC_ADEVS_IO_TYPE>* hybrid_model_oracle =
		new Hybrid<OMC_ADEVS_IO_TYPE>(
		test_oracle,
		new rk_45<OMC_ADEVS_IO_TYPE>(test_oracle,1E-6,0.001),
		new linear_event_locator<OMC_ADEVS_IO_TYPE>(test_oracle,1E-6));
	// Combine them
	SimpleDigraph<double>* model = new SimpleDigraph<double>();
	model->add(hybrid_model);
	model->add(hybrid_model_oracle);
	model->couple(hybrid_model,hybrid_model_oracle);
	model->couple(hybrid_model_oracle,hybrid_model);
	// Create the simulator
	Simulator<OMC_ADEVS_IO_TYPE>* sim =
		new Simulator<OMC_ADEVS_IO_TYPE>(model);
	assert(fabs(pendulum->get_$Ptheta()) < 1E-6);
	cout << "# time, x, y" << endl;
	while (sim->nextEventTime() <= 2.5)
	{
		cout << sim->nextEventTime() << " ";
		sim->execNextEvent();
		cout << pendulum->get_$Px() << " "  
		<< pendulum->get_$Py() << " "
		<< pendulum->get_$Ptheta() << " " 
		<< hybrid_model_oracle->getState(0) << endl;
	}
	assert(test_oracle->getTestCount() > 0);
	delete sim;
	delete model;
	return 0;
}
