#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"

using Coupled = adevs::Coupled<ObjectPtr>;
using Simulator = adevs::Simulator<ObjectPtr>;

int main() {
    std::cout << "Test 1" << std::endl;
    auto model = std::make_shared<Coupled>();
    std::shared_ptr<gcd> c = std::make_shared<gcd>(10.0, 2.0, 1, false);
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 1, true);
    model->add_atomic(g);
    model->add_coupled_model(c);
    model->create_coupling(g->signal, c->in);
    Simulator sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    std::cout << "Test done" << std::endl;
    return 0;
}
