#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 3" << endl;
    auto model = std::make_shared<adevs::Coupled<ObjectPtr>>();
    auto c1 = std::make_shared<gcd>(10, 2, 1, false);
    auto c2 = std::make_shared<gcd>(10, 2, 1, false);
    auto g = std::make_shared<genr>(1, 1, true);
    model->add_atomic(g);
    model->add_coupled_model(c1);
    model->add_coupled_model(c2);
    model->create_coupling(g->signal, c1->in);
    model->create_coupling(c1->out, c2->in);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
