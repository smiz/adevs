#include <iostream>
#include "adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 3" << endl;
    gcd* c1 = new gcd(10, 2, 1, false);
    gcd* c2 = new gcd(10, 2, 1, false);
    genr* g = new genr(1, 1, true);
    adevs::Digraph<object*> model;
    model.add(c1);
    model.add(c2);
    model.add(g);
    model.couple(g, g->signal, c1, c1->in);
    model.couple(c1, c1->out, c2, c2->in);
    adevs::Simulator<PortValue> sim(&model);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    cout << "Test done" << endl;
    return 0;
}
