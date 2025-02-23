#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 2" << endl;
    auto model = std::make_shared<adevs::Graph<ObjectPtr>>();
    vector<double> pat;
    pat.push_back(0);
    pat.push_back(0);
    auto c = std::make_shared<gcd>(*model, 10, 2, 1, false);
    auto g = std::make_shared<genr>(pat, 2, true);
    g->signal = model->add_pin();
    g->start = model->add_pin();
    g->stop = model->add_pin();
    model->add_atomic(g);
    model->connect(g->signal, c->in);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
