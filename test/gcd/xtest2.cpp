#include <algorithm>
#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"
using namespace std;

class generatorEventListener : public adevs::EventListener<ObjectPtr> {
  public:
    void stateChange(adevs::Atomic<ObjectPtr>& model, double) {}
    void inputEvent(adevs::Atomic<ObjectPtr>& model, adevs::PinValue<ObjectPtr>& x, double) {}
    void outputEvent(adevs::Atomic<ObjectPtr>& model, adevs::PinValue<ObjectPtr>& x, double) {
        auto event = pair<adevs::Atomic<ObjectPtr>&,adevs::PinValue<ObjectPtr>>(model, x);
        output.push_back(event);
    }
    vector<pair<adevs::Atomic<ObjectPtr>&,adevs::PinValue<ObjectPtr>>> output;
};

int main() {
    cout << "Test 2x" << endl;
    auto model_g = std::make_shared<adevs::Graph<ObjectPtr>>();
    auto model_c = std::make_shared<adevs::Graph<ObjectPtr>>();
    auto c = std::make_shared<gcd>(*model_c, 10.0, 2.0, 1, false);
    auto g = std::make_shared<genr>(10.0, 1, true);
    model_g->add_atomic(g);
    g->signal = model_g->add_pin();
    auto listener = std::make_shared<generatorEventListener>();
    adevs::Simulator<ObjectPtr> sim_c(model_c);
    adevs::Simulator<ObjectPtr> sim_g(model_g);
    sim_g.addEventListener(listener);
    while (sim_c.nextEventTime() < adevs_inf<double>() || sim_g.nextEventTime() < adevs_inf<double>()) {
        double tN = min(sim_c.nextEventTime(), sim_g.nextEventTime());
        if (sim_g.nextEventTime() == tN) {
            sim_g.computeNextOutput();
        }
        if (sim_c.nextEventTime() == tN) {
            sim_c.computeNextOutput();
        }
        auto iter = listener->output.begin();
        for (; iter != listener->output.end(); iter++) {
            assert(&((*iter).first) == g.get());
            if ((*iter).second.pin == g->signal) {
                adevs::PinValue<ObjectPtr> event;
                event.pin = c->in;
                event.value = (*iter).second.value;
                sim_c.injectInput(event);
            }
        }
        listener->output.clear();
        sim_c.setNextTime(tN);
        assert(sim_c.computeNextState() == tN + adevs_epsilon<double>());
        sim_g.setNextTime(tN);
        assert(sim_g.computeNextState() == tN + adevs_epsilon<double>());
    }
    cout << "Test done" << endl;
    return 0;
}
