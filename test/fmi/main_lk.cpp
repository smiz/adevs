#include "adevs.h"
#include "adevs_fmi.h"
#include "adevs_trap.h"
#include "lk/modelDescription.h"
#include <iostream>
#include <vector>
#include <map>
using namespace std;
using namespace adevs;

void test(ode_system<double>* sys, adevs::ode_solver<double>* solver,
	std::vector<std::pair<double,double> >& traj)
{
	double *q = new double[sys->numVars()];
	const double h = 0.01;
	sys->init(q);
	for (double t = h; t < 50.0; t += h)
	{
		solver->advance(q,h);
		cout << t;
		traj.push_back(std::pair<double,double>(q[0],q[1]));
		for (int i = 0; i < sys->numVars(); i++)
			cout << " " << q[i];
		cout << endl;
	}
	delete [] q;
	delete solver;
	delete sys;
}

int main()
{
	std::vector<std::pair<double,double> > t1, t2;
	lk* model = new lk();
	adevs::trap<double>* trap_solver = new adevs::trap<double>(model,1E-4,0.01);
	test(model,trap_solver,t1);
	model = new lk();
	adevs::rk_45<double>* rk_solver = new adevs::rk_45<double>(model,1E-8,0.01);
	test(model,rk_solver,t2);
	assert(t1.size() == t2.size());
	for (unsigned i = 0; i < t1.size(); i++)
	{
		assert(fabs(t1[i].first-t2[i].first) < 1E-3);
		assert(fabs(t1[i].second-t2[i].second) < 1E-3);
	}
	return 0;
}
