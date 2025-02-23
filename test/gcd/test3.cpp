#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 3" << endl;
    auto model = std::make_shared<adevs::Graph<ObjectPtr>>();
    auto c1 = std::make_shared<gcd>(*model, 10, 2, 1, false);
    auto c2 = std::make_shared<gcd>(*model, 10, 2, 1, false);
    auto g = std::make_shared<genr>(1, 1, true);
    g->signal = model->add_pin();
    model->add_atomic(g);
    model->connect(g->signal, c1->in);
    model->connect(c1->out, c2->in);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
