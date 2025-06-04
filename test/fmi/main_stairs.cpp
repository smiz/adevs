#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"
#include "stairs/modelDescription.h"
using namespace std;
using namespace adevs;

static double const epsilon = 1E-7;
static double const err_tol = 1E-3;

int main() {
    stairs* fmi = new stairs();
    shared_ptr<Hybrid<double>> hybrid_model = make_shared<Hybrid<double>>(
        fmi, new corrected_euler<double>(fmi, epsilon, 0.001),
        new discontinuous_event_locator<double>(fmi, epsilon));
    // Create the simulator
    Simulator<double>* sim = new Simulator<double>(hybrid_model);
    // Run the simulation, testing the solution as we go
    while (sim->nextEventTime() <= 20.5) {
        sim->execNextEvent();
        assert(floor(fmi->get_x()) == fmi->get_step());
    }
    delete sim;
    return 0;
}
