#include "Computer.h"
#include <cassert>
using namespace adevs;

Computer::Computer(double freq) : Network<SimEvent>(), p(), i(freq) {
    p.setParent(this);
    i.setParent(this);
}

void Computer::getComponents(Set<Devs<SimEvent>*> &c) {
    c.insert(&i);
    c.insert(&p);
}

void Computer::route(SimEvent const &value, Devs<SimEvent>* model,
                     Bag<Event<SimEvent>> &r) {
    // Packets and interrupts go to the packet processing model
    if (value.getType() == SIM_PACKET || value.getType() == SIM_INTERRUPT) {
        r.insert(Event<SimEvent>(&p, value));
    }
    // Motor on times go to the interrupt handler
    else if (value.getType() == SIM_MOTOR_ON_TIME) {
        r.insert(Event<SimEvent>(&i, value));
    }
    // Motor voltages are external outputs
    else if (value.getType() == SIM_MOTOR_VOLTAGE) {
        r.insert(Event<SimEvent>(this, value));
    }
    // Any other type is an error
    else {
        assert(false);
    }
}
