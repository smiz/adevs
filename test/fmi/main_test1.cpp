#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"

using Hybrid = adevs::Hybrid<int>;
using ModelExchange = adevs::ModelExchange<int>;
using Simulator = adevs::Simulator<int>;
using corrected_euler = adevs::corrected_euler<int>;
using bisection_event_locator = adevs::bisection_event_locator<int>;

int main() {
    double err_tol = 1E-6;
    auto fmi = new ModelExchange("test1.fmu", err_tol);
    double* J = new double[fmi->numVars() * fmi->numVars()];
    double* q = new double[fmi->numVars()];
    corrected_euler* solver1 = new corrected_euler(fmi, 1E-6, 0.001);
    bisection_event_locator* solver2 = new bisection_event_locator(fmi, 1E-7);
    std::shared_ptr<Hybrid> model = std::make_shared<Hybrid>(fmi, solver1, solver2);
    Simulator* sim = new Simulator(model);
    assert(sim->nextEventTime() < 10.0);
    while (sim->nextEventTime() < 10.0) {
        double t = q[1] = fmi->get_time();
        double x = q[0] = std::any_cast<double>(fmi->get_variable("x"));
        double a = std::any_cast<double>(fmi->get_variable("a"));
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
