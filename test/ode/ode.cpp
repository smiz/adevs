#include "adevs_trap.h"
#include "adevs_rk_45.h"
#include <iostream>
using namespace adevs;
using namespace std;

/**
 * A simple ODE to test numerical integration methods.
 */
class simple_system:
	public ode_system<int>
{
	public:
		simple_system():
		ode_system<int>(2,0)
		{
		}
		void init(double* q)
		{
			q[0] = 1.0;
			q[1] = 0.0;
		}
		void der_func(const double* q, double* dq)
		{
			dq[0] = -q[0]+q[1];
			dq[1] = -q[0]-2.0*q[1];
		}
		void state_event_func(const double* q, double* z){}
		double time_event_func(const double* q) { return adevs_inf<double>(); }
		void internal_event(double* q, const bool* event_flag){}
		void external_event(double* q, double e, const Bag<int>& xb){}
		void confluent_event(double* q, const bool* event_flag, const Bag<int>& xb){}
		void output_func(const double* q, const bool* event_flag,
				Bag<int>& yb){}
		void gc_output(Bag<int>& g){}
		bool get_jacobian(const double* q, double* J)
		{
			if (J == NULL)
				return true;
			J[0] = -1.0;
			J[1] = -1.0;
			J[2] = 1.0;
			J[3] = -2.0;
			return true;
		}
};

/**
 * Non linear lotke-voltair to to test numerical integration methods.
 */
class lk_system:
	public ode_system<int>
{
	private:
		const double a, b, c, d;
	public:
		lk_system():
		ode_system<int>(2,0),
		a(0.1),
		b(0.002),
		c(0.2),
		d(0.0025)
		{
		}
		void init(double* q)
		{
			q[0] = 80.0;
			q[1] = 20.0;
		}
		void der_func(const double* q, double* dq)
		{
			dq[0] = a*q[0]-b*q[0]*q[1];
			dq[1] = d*q[0]*q[1]-c*q[1];
		}
		void state_event_func(const double* q, double* z){}
		double time_event_func(const double* q) { return adevs_inf<double>(); }
		void internal_event(double* q, const bool* event_flag){}
		void external_event(double* q, double e, const Bag<int>& xb){}
		void confluent_event(double* q, const bool* event_flag, const Bag<int>& xb){}
		void output_func(const double* q, const bool* event_flag,
				Bag<int>& yb){}
		void gc_output(Bag<int>& g){}
		bool get_jacobian(const double* q, double* J)
		{
			if (J == NULL)
				return true;
			J[0] = a-b*q[1];
			J[1] = d*q[1];
			J[2] = -b*q[0];
			J[3] = d*q[0]-c;
			return true;
		}
};


void test(ode_system<int>* sys, double tend)
{
	double *q_trap = new double[sys->numVars()];
	double *q_rk = new double[sys->numVars()];
	const double h = 0.01;
	adevs::trap<int>* trap_solver = new adevs::trap<int>(sys,1E-4,0.01);
	adevs::rk_45<int>* rk_solver = new adevs::rk_45<int>(sys,1E-8,0.01);
	sys->init(q_trap);
	sys->init(q_rk);
	for (double t = h; t < tend; t += h)
	{
		trap_solver->advance(q_trap,h);
		rk_solver->advance(q_rk,h);
		cout << t;
		for (int i = 0; i < sys->numVars(); i++)
		{
			cout << " " << q_trap[i] << " " << q_rk[i] << " " << (q_trap[i]-q_rk[i]);
			// Solutions should be very close
			assert(fabs(q_trap[i]-q_rk[i]) < 1E-4);
		}
		cout << endl;
	}
	delete [] q_trap;
	delete [] q_rk;
	delete trap_solver;
	delete rk_solver;
	delete sys;
}

int main()
{
	//test(new simple_system(),1.0);
	test(new lk_system(),100.0);
	return 0;
}
