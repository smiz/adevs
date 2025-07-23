#include <iostream>
#include <map>
#include <vector>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"
#include "adevs/solvers/trap.h"
using namespace std;
using namespace adevs;

void test(ode_system<double>* sys, adevs::ode_solver<double>* solver,
          std::vector<std::pair<double, double>> &traj) {
    double* q = new double[sys->numVars()];
    double const h = 0.01;
    sys->init(q);
    for (double t = h; t < 50.0; t += h) {
        solver->advance(q, h);
        cout << t;
        traj.push_back(std::pair<double, double>(q[0], q[1]));
        for (int i = 0; i < sys->numVars(); i++) {
            cout << " " << q[i];
        }
        cout << endl;
    }
    delete[] q;
    delete solver;
    delete sys;
}

int main() {
    std::vector<std::pair<double, double>> t1, t2;
    auto model = new adevs::ModelExchange<double>("lk.fmu",1E-6);
    adevs::trap<double>* trap_solver =
        new adevs::trap<double>(model, 1E-4, 0.01);
    test(model, trap_solver, t1);
    model = new adevs::ModelExchange<double>("lk.fmu",1E-6);
    adevs::rk_45<double>* rk_solver =
        new adevs::rk_45<double>(model, 1E-8, 0.01);
    test(model, rk_solver, t2);
    assert(t1.size() == t2.size());
    for (unsigned i = 0; i < t1.size(); i++) {
        assert(fabs(t1[i].first - t2[i].first) < 1E-3);
        assert(fabs(t1[i].second - t2[i].second) < 1E-3);
    }
    return 0;
}
