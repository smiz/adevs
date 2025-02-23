#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 7" << endl;
    auto model = std::make_shared<adevs::Graph<ObjectPtr>>();
    auto c = std::make_shared<gcd>(*model, 10, 2, 1, false);
    auto g = std::make_shared<gcd>(*model, 50, 2, 1000, true);
    model->connect(g->signal, c->in);
    model->connect(c->out, g->stop);
    adevs::Simulator<ObjectPtr> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
