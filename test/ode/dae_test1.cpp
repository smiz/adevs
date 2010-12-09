#include "adevs.h"
#include "sampler.h"
#include "check_ball1d_solution.h"
#include <iostream>
#include <typeinfo>
using namespace std;
using namespace adevs;

/**
 * Solves dx/dt = (k+1)x by solving the DAE
 * dx/dt = -y+x, kx-y/4 = 0. This should
 * give a solution x(t) = x(0)exp((1-4k)t).
 * The system starts at x(0)=1 and has state
 * events at x*y = 4kx^2 = 0.4 (so x = 0.01/sqrt(k)).
 * This event causes it to omit an output.
 */
class dae:
	public dae_se1_system<double>
{
	public:
		dae(double k):
		dae_se1_system<double>(2,1,1),
		k(k),
		tripped(false)
		{
		}
		void init(double* q, double* a)
		{
			q[0] = 1.0;
			q[1] = 0.0; // Time
			a[0] = 4.0*k*q[0];
		}
		void alg_func(const double* q, const double* a, double* af)
		{
			af[0] = k*q[0]-0.25*a[0]; 
		}
		void der_func(const double* q, const double* a, double* dq)
		{
			dq[1] = 1.0;
			dq[0] = -a[0]+q[0];
		}
		void state_event_func(const double* q, const double* a, double* z)
		{
			if (tripped)
				z[0] = 1.0;
			else
				z[0] = q[0]*a[0];
		}
		double time_event_func(const double* q, const double* a)
		{
			check_soln(q,a);
			return DBL_MAX;
		}
		void internal_event(double* q, double* a, const bool* event_flag)
		{
			check_soln(q,a);
			double z[1];
			state_event_func(q,a,z);
			assert(fabs(z[0]) < 1E-5);
		}
		void postStep(const double* q, const double* a)
		{
			check_soln(q,a);
		}
		void external_event(double* q, double* a, double e, const Bag<double>& xb)
		{
			check_soln(q,a);
		}
		void confluent_event(double* q, double* a, const bool* event_flag, const Bag<double>& xb)
		{
			internal_event(q,a,event_flag);
		}
		void output_func(const double* q, const double* a, const bool* event_flag,
				Bag<double>& yb)
		{
			check_soln(q,a);
			double z[1];
			state_event_func(q,a,z);
			assert(fabs(z[0]) < 1E-5);
			yb.insert(1.0);
		}
		void gc_output(Bag<double>& g){}

	private:
		const double k;
		bool tripped;

		void check_soln(const double* q, const double* a)
		{
			assert(fabs(k*q[0]-0.25*a[0]) < 1E-5);
			assert(fabs(exp((1.0-4.0*k)*q[1])-q[0]) < 1E-5);
		}
};

class SolutionPlotter:
	public EventListener<double>
{
	public:
		SolutionPlotter(Hybrid<double>* hsys, dae_se1_system<double>* dsys):
			hsys(hsys),
			dsys(dsys)
			{}
		void stateChange(Atomic<double>* model, double t)
		{
			cout << t << " " << hsys->getState(1) << " " << hsys->getState(0) << " " << dsys->getAlgVar(0) << endl;
		}
	private:
		Hybrid<double>* hsys;
		dae_se1_system<double>* dsys;
};

void run_test(dae_se1_system<double>* b,
	ode_solver<double> *s,
	event_locator<double>* l)
{
	Hybrid<double>* model =
		new Hybrid<double>(b,s,l);
	SolutionPlotter* plotter = new SolutionPlotter(model,b);
	Simulator<double>* sim = new Simulator<double>(model);
	sim->addEventListener(plotter);
	while (sim->nextEventTime() < 1.0)
	{
		sim->execNextEvent();
	}
	delete sim;
	delete model;
	delete plotter;
}

int main()
{
	dae_se1_system<double>* sys = new dae(1.0);
	run_test(sys,new rk_45<double>(sys,1E-6,0.01),
			new linear_event_locator<double>(sys,1E-7));
	return 0;
}
