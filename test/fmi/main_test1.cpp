#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"
#include "test1/modelDescription.h"
using namespace std;

int main() {
    test1* fmi = new test1();
    double* J = new double[fmi->numVars() * fmi->numVars()];
    double* q = new double[fmi->numVars()];
    adevs::corrected_euler<int>* solver1 =
        new adevs::corrected_euler<int>(fmi, 1E-6, 0.001);
    adevs::bisection_event_locator<int>* solver2 =
        new adevs::bisection_event_locator<int>(fmi, 1E-7);
    shared_ptr<adevs::Hybrid<int>> model = make_shared<adevs::Hybrid<int>>(fmi, solver1, solver2);
    adevs::Simulator<int>* sim = new adevs::Simulator<int>(model);
    assert(sim->nextEventTime() < 10.0);
    while (sim->nextEventTime() < 10.0) {
        double t = q[1] = fmi->get_time();
        double x = q[0] = fmi->get_x();
        double a = fmi->get_a();
        // Solution error
        double err = fabs(x - exp(a * t));
        assert(err < 1E-3);
        // Jacobian values
        fmi->get_jacobian(q, J);
        assert(J[0] == a);
        assert(J[1] == 0.0);
        assert(J[2] == 0.0);
        assert(J[3] == 1.0);
        sim->execNextEvent();
    }
    assert(sim->nextEventTime() >= 10.0);
    delete sim;
    return 0;
}
