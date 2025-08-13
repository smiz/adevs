#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 1" << endl;
    auto model = std::make_shared<adevs::Coupled<ObjectPtr>>();
    shared_ptr<gcd> c = std::make_shared<gcd>(10.0, 2.0, 1, false);
    shared_ptr<genr> g = std::make_shared<genr>(10.0, 1, true);
    model->add_atomic(g);
    model->add_coupled_model(c);
    model->create_coupling(g->signal, c->in);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
