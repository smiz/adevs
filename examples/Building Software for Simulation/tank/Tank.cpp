#include "Tank.h"
#include <cassert>
#include <iostream>
using namespace std;
using namespace adevs;

Tank::Tank(double freq, double x0, double y0, double theta0, double cint)
    : Network<SimEvent>(),
      computer(freq),
      physics(new TankPhysicsEqns(x0, y0, theta0, cint, false)) {
    computer.setParent(this);
    physics.setParent(this);
}

void Tank::getComponents(Set<Devs<SimEvent>*> &c) {
    c.insert(&computer);
    c.insert(&physics);
}

void Tank::route(SimEvent const &value, Devs<SimEvent>* model,
                 Bag<Event<SimEvent>> &r) {
    // Packets go to the computer
    if (value.getType() == SIM_PACKET) {
        r.insert(Event<SimEvent>(&computer, value));
    }
    // Voltage events go to the tank physics model
    else if (value.getType() == SIM_MOTOR_VOLTAGE) {
        r.insert(Event<SimEvent>(&physics, value));
    }
    // Position events are external output
    else if (value.getType() == SIM_TANK_POSITION) {
        r.insert(Event<SimEvent>(this, value));
    }
    // Anything else is an error
    else {
        assert(false);
    }
}
