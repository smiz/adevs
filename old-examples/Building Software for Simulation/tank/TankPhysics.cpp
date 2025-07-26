#include "TankPhysics.h"
using namespace std;
using namespace adevs;

// Get the state variable values
double TankPhysics::leftMotorCurrent() const {
    return eqns->getLeftCurrent(getState());
}

double TankPhysics::rightMotorCurrent() const {
    return eqns->getRightCurrent(getState());
}

double TankPhysics::leftMotorSpeed() const {
    return getState(eqns->WL);
}

double TankPhysics::rightMotorSpeed() const {
    return getState(eqns->WR);
}

double TankPhysics::getSpeed() const {
    return getState(eqns->V);
}

double TankPhysics::getTurnSpeed() const {
    return getState(eqns->W);
}

double TankPhysics::getX() const {
    return getState(eqns->X);
}

double TankPhysics::getY() const {
    return getState(eqns->Y);
}

double TankPhysics::getHeading() const {
    return getState(eqns->THETA);
}

bool TankPhysics::isTurning() const {
    return eqns->isTurning();
}

double TankPhysics::getTorque() const {
    return eqns->B * (getState(eqns->FL) - getState(eqns->FR)) / 2.0;
}

double TankPhysics::getLeftForce() const {
    return getState(eqns->FL);
}

double TankPhysics::getRightForce() const {
    return getState(eqns->FR);
}

TankPhysics::TankPhysics(TankPhysicsEqns* eqns)
    : Hybrid<SimEvent>(eqns, new rk_45<SimEvent>(eqns, 1E-2, 0.1),
                       new linear_event_locator<SimEvent>(eqns, 1E-8)),
      eqns(eqns) {}
