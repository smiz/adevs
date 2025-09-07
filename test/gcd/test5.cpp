#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"


int main() {
    std::cout << "Test 5" << std::endl;
    auto model = std::make_shared<adevs::Coupled<ObjectPtr>>();
    auto c = std::make_shared<gcd>(10, 2, 1, false);
    auto g = std::make_shared<genr>(50, 1000, true);
    model->add_atomic(g);
    model->add_coupled_model(c);
    model->create_coupling(g->signal, c->in);
    model->create_coupling(c->out, g->stop);
    model->create_coupling(g->stop, g);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    std::cout << "Test done" << std::endl;
    return 0;
}
