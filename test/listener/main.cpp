#include "relay.h"

// using namespace adevs;

std::shared_ptr<Relay> r = nullptr;
std::vector<adevs::PinValue<int>> output;
std::vector<adevs::PinValue<int>> input;

class Listener : public adevs::EventListener<int> {
  public:
    Listener() : adevs::EventListener<int>() {}

    void inputEvent(adevs::Atomic<int>& model, adevs::PinValue<int>& x, double t) {
        // First input should occur at time zero
        assert(&model == r.get());
        assert(input.size() == 0 || t == 0.0);
        input.push_back(x);
    }

    void outputEvent(adevs::Atomic<int>& model, adevs::PinValue<int>& x, double t) {
        // Output should occur only at the relay time
        assert(t == 1.0);
        assert(&model == r.get());
        // Save to check its validity
        output.push_back(x);
    }

    void stateChange(adevs::Atomic<int>& model, double t) {
        assert(&model == r.get());
        // First input should set the relay value to something positive
        if (t == 0.0) {
            assert(r->getRelayValue() > 0);
        }
        // Second event should clear the relay value
        else if (t == 1.0) {
            assert(r->getRelayValue() < 0);
        }
        // Anything else is an error
        else {
            assert(false);
        }
    }
};

int main() {
    std::shared_ptr<adevs::Graph<int>> d = std::make_shared<adevs::Graph<int>>();
    r = std::make_shared<Relay>();
    d->add_atomic(r);
    d->connect(r->in, r);
    // Create the simulator and add the listener
    std::shared_ptr<Listener> listener = std::make_shared<Listener>();

    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(d);
    sim->addEventListener(listener);

    // This input should cause two outputEvent() calls at time 1
    adevs::PinValue<int> b(r->in,1);

    // Inject it at time 0
    sim->injectInput(b);
    sim->setNextTime(0.0);
    sim->execNextEvent();

    // Next event at time 1?
    assert(sim->nextEventTime() == 1.0);

    // No output events!
    assert(output.size() == 0);

    // Execute the next event
    sim->execNextEvent();

    // Should be one output and one input
    assert(output.size() == 1);
    assert(input.size() == 1);

    // Check the output sources
    assert(output[0].pin == r->out);
    assert(input[0].pin == r->in);
    // Done
    return 0;
}
