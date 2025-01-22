#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 1" << endl;
    adevs::Digraph<object*> model;
    shared_ptr<gcd> c = make_shared<gcd>(10.0, 2.0, 1, false);
    shared_ptr<genr> g = make_shared<genr>(10.0, 1, true);
    model.couple(g, g->signal, c, c->in);
    adevs::Simulator<PortValue> sim(model);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
