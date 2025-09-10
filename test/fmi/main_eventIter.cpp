#include <cassert>
#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"

// using namespace adevs;
using ModelExchange = adevs::ModelExchange<>;
using Simulator = adevs::Simulator<>;
using ExplicitHybrid = ExplicitHybrid<>;

static double const epsilon = 1E-3;

void print(ModelExchange* model) {
    double c = std::any_cast<double>(model->get_variable("c"));
    int high = std::any_cast<int>(model->get_variable("high"));
    double floor_c = std::any_cast<double>(model->get_variable("floorc"));
    double cfloor = c - floor(c);
    std::cout << model->get_time() << " " << c << " "
         << floor_c << " " << high << std::endl;
    assert(cfloor <= 0.5 + epsilon);
    assert(cfloor <= 0.25 + epsilon || high == 1);
    assert(cfloor >= 0.25 - epsilon || high == 0);
    assert(floor_c == cfloor);
}

int main() {
    const double err_tol = 1E-6;
    auto test_model = new ModelExchange("eventIter.fmu",err_tol);
    auto model = std::make_shared<ExplicitHybrid>(test_model,err_tol,0.001);
    // Create the simulator
    Simulator sim(model);
    // Check initial values
    print(test_model);
    // Run the simulation, testing the solution as we go
    while (sim.nextEventTime() <= 5.0) {
        sim.execNextEvent();
        print(test_model);
    }
}
