#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"

using Coupled = adevs::Coupled<ObjectPtr>;
using Simulator = adevs::Simulator<ObjectPtr>;

int main() {
    std::cout << "Test 4" << std::endl;
    std::vector<double> pat;
    pat.push_back(0);
    pat.push_back(0);
    auto model = std::make_shared<Coupled>();
    auto c1 = std::make_shared<gcd>(10, 2, 1, false);
    auto c2 = std::make_shared<gcd>(10, 2, 1, false);
    auto g = std::make_shared<genr>(pat, 2, true);
    model->add_atomic(g);
    model->add_coupled_model(c1);
    model->add_coupled_model(c2);
    model->create_coupling(g->signal, c1->in);
    model->create_coupling(c1->out, c2->in);
    Simulator sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    std::cout << "Test done" << std::endl;
    return 0;
}
