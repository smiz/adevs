#include <iostream>
#include <memory>
#include "adevs/adevs.h"
#include "gcd.h"

using namespace std;

int main() {
    cout << "Test 1x" << endl;
    shared_ptr<adevs::Digraph<object*>> model =
        make_shared < adevs::Digraph<object*>();
    shared_ptr<gcd> c = make_shared<gcd>(10.0, 2.0, 1, false);
    shared_ptr<genr> g = make_shared<genr>(10.0, 1, true);
    model->couple(g, g->signal, c, c->in);
    adevs::Simulator<PortValue> sim(model);
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
