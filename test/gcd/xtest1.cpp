#include <iostream>
#include "adevs.h"
#include "gcd.h"
using namespace std;

int main() {
    cout << "Test 1x" << endl;
    adevs::Digraph<object*> model;
    gcd* c = new gcd(10.0, 2.0, 1, false);
    genr* g = new genr(10.0, 1, true);
    model.couple(g, g->signal, c, c->in);
    adevs::Simulator<PortValue> sim(&model);
    adevs::Bag<adevs::Event<PortValue>> input;
    while (sim.nextEventTime() < DBL_MAX) {
        double tnew = sim.nextEventTime();
        sim.computeNextOutput();
        assert(sim.computeNextState(input, sim.nextEventTime()) ==
               tnew + adevs_epsilon<double>());
    }
    cout << "Test done" << endl;
    return 0;
}
