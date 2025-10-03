#include <algorithm>
#include <iostream>
#include "adevs/adevs.h"
#include "gcd.h"

using Atomic = adevs::Atomic<ObjectPtr>;
using Simulator = adevs::Simulator<ObjectPtr>;
using EventListener = adevs::EventListener<ObjectPtr>;

class generatorEventListener : public EventListener {
  public:
    void stateChange(Atomic &, double) {}
    void inputEvent(Atomic &, PinValue &, double) {}
    void outputEvent(Atomic &model, PinValue &x, double) {
        auto event = std::pair<Atomic &, PinValue>(model, x);
        output.push_back(event);
    }
    std::vector<std::pair<Atomic &, PinValue>> output;
};

int main() {
    std::cout << "Test 2x" << std::endl;
    auto c = std::make_shared<gcd>(10.0, 2.0, 1, false);
    auto g = std::make_shared<genr>(10.0, 1, true);
    auto listener = std::make_shared<generatorEventListener>();
    Simulator sim_c(c);
    Simulator sim_g(g);
    sim_g.addEventListener(listener);
    while (sim_c.nextEventTime() < adevs_inf<double>() ||
           sim_g.nextEventTime() < adevs_inf<double>()) {
        double tN = std::min(sim_c.nextEventTime(), sim_g.nextEventTime());
        sim_c.setNextTime(tN);
        sim_g.setNextTime(tN);
        sim_g.computeNextOutput();
        auto iter = listener->output.begin();
        for (; iter != listener->output.end(); iter++) {
            assert(&((*iter).first) == g.get());
            if ((*iter).second.pin == g->signal) {
                PinValue event;
                event.pin = c->in;
                event.value = (*iter).second.value;
                sim_c.injectInput(event);
            }
        }
        listener->output.clear();
        sim_c.computeNextOutput();
        assert(sim_c.computeNextState() == tN + adevs_epsilon<double>());
        assert(sim_g.computeNextState() == tN + adevs_epsilon<double>());
    }
    std::cout << "Test done" << std::endl;
    return 0;
}
