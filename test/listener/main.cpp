#include <vector>
#include "Relay.h"
using namespace std;
using namespace adevs;

Relay* r;
vector<Event<IO_Type>> output;
vector<Event<IO_Type>> input;

class Listener : public EventListener<IO_Type> {
  public:
    Listener() : EventListener<IO_Type>() {}
    void inputEvent(Event<IO_Type> x, double t) {
        // First input should occur at time zero
        assert(input.size() == 0 || t == 0.0);
        input.push_back(x);
    }
    void outputEvent(Event<IO_Type> x, double t) {
        // Output should occur only at the relay time
        assert(t == 1.0);
        // Save to check its validity
        output.push_back(x);
    }
    void stateChange(Atomic<IO_Type>* model, double t) {
        assert(model == r);
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
    Digraph<int>* d = new Digraph<int>();
    r = new Relay();
    d->add(r);
    d->couple(d, 0, r, 0);
    d->couple(r, 1, d, 1);
    // Create the simulator and add the listener
    Simulator<IO_Type>* sim = new Simulator<IO_Type>(d);
    sim->addEventListener(new Listener());
    // This input should cause two outputEvent() calls at time 1
    Bag<Event<IO_Type>> b;
    b.push_back(Event<IO_Type>(d, IO_Type(0, 1)));
    // Inject it at time 0
    sim->computeNextState(b, 0.0);
    // Next event at time 1?
    assert(sim->nextEventTime() == 1.0);
    // No output events!
    assert(output.size() == 0);
    // Execute the next event
    sim->execNextEvent();
    // Should be two outputs and one input
    assert(output.size() == 2);
    assert(input.size() == 1);
    // Check the output sources
    assert(output[0].model == d || output[1].model == d);
    assert(output[0].model == r || output[1].model == r);
    // Done
    return 0;
}
