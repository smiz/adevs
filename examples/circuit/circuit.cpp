#include <iostream>
#include "adevs/adevs.h"
using namespace std;
using namespace adevs;

/**
 * This is the circuit example from "Building Software for Simulation". It
 * prints the trajectory of the model to stdout; if this is captured in a file,
 * then the gnuplot command file 'plot' can be used to plot the voltage and
 * diode and switch states.
 */
class Circuit : public ode_system<bool> {
  public:
    Circuit()
        : ode_system<bool>(1, 1),  // one state variable and event function
          s(1),
          d(0),  // diode and switch states
          vs(1.0),
          C(1.0),
          Rs(1.0),
          Rl(1.0),  // device parameters
          vop(0.5),
          vcl(0.25) {}
    void init(double* q) { q[0] = 0.0; }  // vc = 0
    void der_func(double const* q, double* dq) {
        // ODE form of the differential equations
        if (!s && !d) {
            dq[0] = 0.0;
        } else if (!s && d) {
            dq[0] = -q[0] / (C * Rl);
        } else if (s && !d) {
            dq[0] = (vs - q[0]) / (C * Rs);
        } else {
            dq[0] = ((vs - q[0]) / Rs - q[0] / Rl) / C;
        }
    }
    void state_event_func(double const* q, double* z) {
        // This model uses the implicit form of the diode event
        if (d == 0) {
            z[0] = q[0] - vop;
        } else {
            z[0] = q[0] - vcl;
        }
    }
    // As written here, this model does not have any time events
    double time_event_func(double const* q) { return DBL_MAX; }
    void internal_event(double* q, bool const* events) {
        assert(events[0]);  // only one event type; make sure it fired
        d = !d;
    }
    void external_event(double*, double, list<bool> const &xb) {
        s = *(xb.begin());
    }
    void confluent_event(double* q, bool const* events, list<bool> const &xb) {
        internal_event(q, events);
        external_event(q, 0.0, xb);
    }
    void output_func(double const* q, bool const* events, list<bool> &yb) {
        assert(events[0]);
        yb.push_back(!d);
    }

    bool getDiode() const { return d; }
    bool getSwitch() const { return s; }

  private:
    bool s, d;
    double const vs, C, Rs, Rl, vop, vcl;
};

/// A switch that opens at t_open
class OpenSwitch : public Atomic<bool> {
  public:
    OpenSwitch(double t_open) : Atomic<bool>(), t_open(t_open) {}
    double ta() { return t_open; }
    void delta_int() { t_open = adevs_inf<double>(); }
    void delta_ext(double, list<bool> const &) {}
    void delta_conf(list<bool> const &) {}
    void output_func(list<bool> &yb) { yb.push_back(false); }


  private:
    double t_open;
};

int main() {
    // Create the model
    Circuit* circuit = new Circuit();
    Hybrid<bool>* hybrid_model = new Hybrid<bool>(
        circuit, new corrected_euler<bool>(circuit, 1E-5, 0.01),
        new linear_event_locator<bool>(circuit, 1E-5));
    OpenSwitch* open_switch = new OpenSwitch(1.0);
    SimpleDigraph<bool>* model = new SimpleDigraph<bool>();
    model->add(hybrid_model);
    model->add(open_switch);
    model->couple(open_switch, hybrid_model);
    // Create the simulator
    Simulator<bool>* sim = new Simulator<bool>(model);
    // Simulate until the switch and diode have both experienced
    // an event
    double tNow = 0.0;
    while (sim->nextEventTime() <= 4.0) {
        cout << tNow << " " << hybrid_model->getState(0) << " "
             << circuit->getSwitch() << " " << circuit->getDiode() << endl;
        tNow = sim->nextEventTime();
        sim->execNextEvent();
    }
    delete sim;
    delete model;
    return 0;
}
