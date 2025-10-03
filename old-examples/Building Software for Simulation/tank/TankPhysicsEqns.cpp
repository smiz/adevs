#include "TankPhysics.h"

// using namespace adevs;

// Indices for the state event array
int const TankPhysicsEqns::TURNL = 0, TankPhysicsEqns::TURNR = 1,
          TankPhysicsEqns::TIMER_EXPIRE = 2;
// Indices for the state variable array
int const TankPhysicsEqns::WL = 0, TankPhysicsEqns::FL = 1,
          TankPhysicsEqns::IL = 11, TankPhysicsEqns::WR = 3,
          TankPhysicsEqns::FR = 4, TankPhysicsEqns::IR = 10,
          TankPhysicsEqns::X = 6, TankPhysicsEqns::Y = 7,
          TankPhysicsEqns::THETA = 8, TankPhysicsEqns::W = 9,
          TankPhysicsEqns::V = 5, TankPhysicsEqns::TIMER = 2;

TankPhysicsEqns::TankPhysicsEqns(double x0, double y0, double theta0,
                                 double cint, bool simple)
    :  // The simple model has two fewer state variables
      ode_system<SimEvent>(12 - simple * 2, 2),
      // Set the model parameters and initial conditions
      mt(0.8),
      Jt(5E-4),
      B(0.1),
      Br(1.0),
      Bs(14.0),
      Bl(0.7),
      Sl(0.3),
      Lm(1E-3),
      Rm(3.1),
      Jg(1.2E-6),
      Bg(6.7E-7),
      g(204.0),
      alpha(1E-3),
      r(0.015),
      Kt(0.001),
      x0(x0),
      y0(y0),
      theta0(theta0),
      Hs(1E-3),
      cint(cint),
      el(0.0),
      er(0.0),
      turning(false),
      simple(simple) {}

void TankPhysicsEqns::init(double* q) {
    q[W] = q[V] = q[FR] = q[FL] = q[WR] = q[WL] = 0.0;
    if (!simple) {
        q[IL] = q[IR] = 0.0;
    }
    q[X] = x0;
    q[Y] = y0;
    q[THETA] = theta0;
    q[TIMER] = cint;
}

void TankPhysicsEqns::der_func(double const* q, double* dq) {
    double il = getLeftCurrent(q), ir = getRightCurrent(q);
    // Timer just counts down
    dq[TIMER] = -1.0;
    // These do not depend directly on if the tank is turning
    if (!simple) {
        dq[IL] = (el - il * Rm - alpha * q[WL]) / Lm;
        dq[IR] = (er - ir * Rm - alpha * q[WR]) / Lm;
    }
    dq[WL] = (alpha * il - q[WL] * Bg - (r / g) * q[FL]) / Jg;
    dq[WR] = (alpha * ir - q[WR] * Bg - (r / g) * q[FR]) / Jg;
    dq[FL] = ((r / g) * q[WL] - (q[V] + B * q[W] / 2.0)) / Kt;
    dq[FR] = ((r / g) * q[WR] - (q[V] - B * q[W] / 2.0)) / Kt;
    dq[X] = q[V] * sin(q[THETA]);
    dq[Y] = q[V] * cos(q[THETA]);
    assert(turning || q[W] == 0.0);
    dq[THETA] = q[W];
    // These equations change when the tank turns or does not turn
    dq[V] = (q[FL] + q[FR] - (Br + Bs * (double)turning) * q[V]) / mt;
    dq[W] = (double)turning * (B * (q[FL] - q[FR]) / 2.0 - Bl * q[W]) / Jt;
}

void TankPhysicsEqns::state_event_func(double const* q, double* z) {
    double torque_l = B * (q[FL] - q[FR]) / 2.0;
    double torque_r = -torque_l;
    z[TURNL] = torque_l - (Sl - (double)turning * Hs);
    z[TURNR] = torque_r - (Sl - (double)turning * Hs);
}

double TankPhysicsEqns::time_event_func(double const* q) {
    return std::max(0.0, q[TIMER]);
}

void TankPhysicsEqns::internal_event(double* q, bool const* events) {
    // Start or end a turn; this produces an output so reset the timer
    if (events[TURNL] || events[TURNR]) {
        q[TIMER] = cint;
        q[W] = 0.0;
        turning = !turning;
    }
    // Otherwise is was a timer event, so just reset the timer
    else {
        q[TIMER] = cint;
    }
}

void TankPhysicsEqns::external_event(double* q, double e,
                                     std::list<SimEvent> const &xb) {
    // Set the motor voltage
    std::list<SimEvent>::iterator iter = xb.begin();
    for (; iter != xb.end(); iter++) {
        assert((*iter).getType() == SIM_MOTOR_VOLTAGE);
        el = (*iter).simMotorVoltage().el;
        er = (*iter).simMotorVoltage().er;
    }
}

void TankPhysicsEqns::confluent_event(double* q, bool const* events,
                                      std::list<SimEvent> const &xb) {
    internal_event(q, events);
    external_event(q, 0.0, xb);
}

void TankPhysicsEqns::output_func(double const* q, bool const* events,
                                  std::list<SimEvent> &yb) {
    // Produce a position event
    SimTankPosition pos;
    pos.x = q[X];
    pos.y = q[Y];
    pos.theta = q[THETA];
    yb.push_back(SimEvent(pos));
}

double TankPhysicsEqns::getLeftCurrent(double const* q) const {
    if (simple) {
        return (el - alpha * q[WL]) / Rm;
    } else {
        return q[IL];
    }
}

double TankPhysicsEqns::getRightCurrent(double const* q) const {
    if (simple) {
        return (er - alpha * q[WR]) / Rm;
    } else {
        return q[IR];
    }
}
