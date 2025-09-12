#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"

using ModelExchange = adevs::ModelExchange<>;
using ExplicitHybrid = adevs::ExplicitHybrid<>;
using Simulator = adevs::Simulator<>;

static double const err_tol = 1E-6;

int main() {
    auto fmu = new ModelExchange("stairs.fmu", err_tol);
    auto model = std::make_shared<ExplicitHybrid>(fmu, err_tol, 0.01);
    // Create the simulator
    Simulator sim(model);
    // Run the simulation, testing the solution as we go
    while (sim.nextEventTime() <= 20.5) {
        sim.execNextEvent();
        double x = std::any_cast<double>(fmu->get_variable("x"));
        double step = std::any_cast<double>(fmu->get_variable("step"));
        assert(floor(x) == step);
    }
    return 0;
}
