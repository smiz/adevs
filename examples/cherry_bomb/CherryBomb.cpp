#include <iostream>
#include "adevs/adevs.h"
using namespace std;
using namespace adevs;

// Array indices for the CherryBomb state variables
#define H 0
#define V 1
#define T 2
// Discrete variable enumeration for the CherryBomb
typedef enum { FUSE_LIT, DOUSE, EXPLODE } Phase;

class CherryBomb : public ode_system<string> {
  public:
    CherryBomb()
        : ode_system<string>(3,  // three state variables including time
                             1   // 1 state event condition
          ) {
        phase = FUSE_LIT;  // Light the fuse!
    }
    void init(double* q) {
        q[H] = 1.0;  // Initial height
        q[V] = 0.0;  // Initial velocity
        q[T] = 0.0;  // Start time at zero
    }
    void der_func(double const* q, double* dq) {
        dq[V] = -9.8;
        dq[H] = q[V];
        dq[T] = 1.0;
    }
    void state_event_func(double const* q, double* z) {
        // Test for hitting the ground.
        if (q[V] < 0.0) {
            z[0] = q[H];
        } else {
            z[0] = 1.0;
        }
    }
    double time_event_func(double const* q) {
        if (q[T] < 2.0) {
            return 2.0 - q[T];  // Explode at time 2
        } else {
            return DBL_MAX;  // Don't do anything after that
        }
    }
    void external_event(double* q, double e, Bag<string> const &xb) {
        phase = DOUSE;  // Any input is a douse event
    }
    void internal_event(double* q, bool const* state_event) {
        if (state_event[0]) {
            q[V] = -q[V];  // Bounce!
        }
        if (state_event[1]) {
            phase = EXPLODE;
        }
    }
    void confluent_event(double* q, bool const* state_event,
                         Bag<string> const &xb) {
        internal_event(q, state_event);
        external_event(q, 0.0, xb);
    }
    void output_func(double const* q, bool const* state_event,
                     Bag<string> &yb) {
        if (state_event[1] && phase == FUSE_LIT) {
            yb.push_back("BOOM!");  // Explode!
        }
    }
    void postStep(double* q) {
        // Write the current state to std out
        cout << q[T] << " " << q[H] << " " << q[V] << " " << phase << endl;
    }
    // No garbage collection is needed
    void gc_output(Bag<string> &) {}
    // Get the current value of the discrete variable
    Phase getPhase() { return phase; }

  private:
    Phase phase;
};

int main() {
    // Create the model
    CherryBomb* bomb = new CherryBomb();
    // Create the ODE solver for this model. Maximum error
    // tolerance at each step is 1E-4 and the maximum
    // size of an integration step is 0.01.
    ode_solver<string>* ode_solve =
        new corrected_euler<string>(bomb, 1E-4, 0.01);
    // Create the event locator for this model. Maximum
    // error tolerace for the location of an event in
    // the state space is 1E-8.
    event_locator<string>* event_find =
        new linear_event_locator<string>(bomb, 1E-8);
    // Create an atomic model that puts all of these
    // together to simulate the continuous system.
    Hybrid<string>* model = new Hybrid<string>(bomb, ode_solve, event_find);
    // Create and run a simulator for this model
    Simulator<string>* sim = new Simulator<string>(model);
    while (bomb->getPhase() == FUSE_LIT) {
        sim->execNextEvent();
    }
    delete sim;
    delete bomb;
    return 0;
}
