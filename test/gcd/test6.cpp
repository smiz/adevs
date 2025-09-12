#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"

using Coupled = adevs::Coupled<ObjectPtr>;
using Simulator = adevs::Simulator<ObjectPtr>;

int main() {
    std::cout << "Test 6" << std::endl;
    std::vector<double> pat;
    pat.push_back(50);
    pat.push_back(0);
    auto model = std::make_shared<Coupled>();
    auto c = std::make_shared<gcd>(10, 2, 1, false);
    auto g = std::make_shared<genr>(pat, 1000, true);
    model->add_atomic(g);
    model->add_coupled_model(c);
    model->create_coupling(g->stop, g);
    model->create_coupling(g->signal, c->in);
    model->create_coupling(c->out, g->stop);
    Simulator sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    std::cout << "Test done" << std::endl;
}
