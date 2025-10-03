#ifndef _TankPhysicsEqns_h_
#define _TankPhysicsEqns_h_
#include <fstream>
#include "SimEvents.h"
#include "adevs/adevs.h"
#include "adevs/hybrid.h"

class TankPhysicsEqns : public adevs::ode_system<SimEvent> {
  public:
    // Create a tank at an intial position
    TankPhysicsEqns(double x0, double y0, double theta0, double cint,
                    bool simple = false);
    // Initialize the continuous state variables
    void init(double* q);
    // Differential functions
    void der_func(double const* q, double* dq);
    // State event function that detects turning movements
    void state_event_func(double const* q, double* z);
    // Sample x, y, and theta periodically
    double time_event_func(double const* q);
    // Expiration of the timer and collision events are internal
    void internal_event(double* q, bool const* events);
    // Change in the motor voltage is an external event
    void external_event(double* q, double e, std::list<SimEvent> const &xb);
    // Confluent events
    void confluent_event(double* q, bool const* events,
                         std::list<SimEvent> const &xb);
    // Output position events for the display when the timer expires
    void output_func(double const* q, bool const* events, std::list<SimEvent> &yb);

    // Get the resistance of the motor (left and right are the same)
    double getMotorOhms() const { return Rm; }
    // Is the tank turning?
    bool isTurning() const { return turning; }
    // Get the current in the motor
    double getLeftCurrent(double const* q) const;
    double getRightCurrent(double const* q) const;
    // Index of the turning state events and timer time event
    static int const TURNL, TURNR, TIMER_EXPIRE;
    // Indices of the state variables
    static int const WL, WR, FL, FR, IL, IR, X, Y, THETA, W, V, TIMER;
    // Model parameters
    double const mt, Jt, B, Br, Bs, Bl, Sl, Lm, Rm, Jg, Bg, g, alpha, r, Kt;

  private:
    // Initial conditions
    double const x0, y0, theta0;
    // Hysteresis value for stopping turns
    double const Hs;
    // Communication interval for the numerical integration algorithm
    double const cint;
    // Motor voltages
    double el, er;
    // Is the tank turning?
    bool turning;
    // Use simplified dynamics?
    bool const simple;
};

#endif
