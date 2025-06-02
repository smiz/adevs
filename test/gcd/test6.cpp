#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 6" << endl;
    vector<double> pat;
    pat.push_back(50);
    pat.push_back(0);
    auto model = std::make_shared<adevs::Graph<ObjectPtr>>();
    auto c = std::make_shared<gcd>(*model, 10, 2, 1, false);
    auto g = std::make_shared<genr>(pat, 1000, true);
    model->add_atomic(g);
    model->connect(g->stop,g);
    model->connect(g->signal, c->in);
    model->connect(c->out, g->stop);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
}
