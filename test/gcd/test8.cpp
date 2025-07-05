#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 8" << endl;
    vector<double> pat;
    pat.push_back(50);
    pat.push_back(0);
    auto model = std::make_shared<adevs::Coupled<ObjectPtr>>();
    auto c = std::make_shared<gcd>(10, 2, 1, false);
    auto g = std::make_shared<gcd>(pat, 2, 1000, true);
    model->add_coupled_model(c);
    model->add_coupled_model(g);
    model->create_coupling(g->signal, c->in);
    model->create_coupling(c->out, g->stop);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
