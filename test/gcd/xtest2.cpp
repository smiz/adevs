#include <algorithm>
#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

class generatorEventListener : public adevs::EventListener<PortValue> {
  public:
    void outputEvent(adevs::Event<PortValue> x, double) { output.push_back(x); }
    vector<adevs::Event<PortValue>> output;
};

int main() {
    cout << "Test 2x" << endl;
    gcd* c = new gcd(10.0, 2.0, 1, false);
    genr* g = new genr(10.0, 1, true);
    generatorEventListener listener;
    adevs::Simulator<PortValue> sim_c(c);
    adevs::Simulator<PortValue> sim_g(g);
    sim_g.addEventListener(&listener);
    while (sim_c.nextEventTime() < DBL_MAX || sim_g.nextEventTime() < DBL_MAX) {
        double tN = min(sim_c.nextEventTime(), sim_g.nextEventTime());
        if (sim_g.nextEventTime() == tN) {
            sim_g.computeNextOutput();
        }
        if (sim_c.nextEventTime() == tN) {
            sim_c.computeNextOutput();
        }
        adevs::Bag<adevs::Event<PortValue>> y;
        vector<adevs::Event<PortValue>>::iterator iter =
            listener.output.begin();
        for (; iter != listener.output.end(); iter++) {
            assert((*iter).model == g);
            if ((*iter).value.port == g->signal) {
                adevs::Event<PortValue> event;
                event.model = c;
                event.value.port = c->in;
                event.value.value = (*iter).value.value;
                y.insert(event);
            }
        }
        listener.output.clear();
        assert(sim_c.computeNextState(y, tN) == tN + adevs_epsilon<double>());
        y.clear();
        assert(sim_g.computeNextState(y, tN) == tN + adevs_epsilon<double>());
    }
    cout << "Test done" << endl;
    return 0;
}
