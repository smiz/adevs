#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/trap.h" // Get the ImplicitHybrid class
using namespace std;

/**
 * This example simulates an electrical circuit with a switch and
 * a diode. The circuit diagram is as shown below.
 *           /
 * +---Rs---/  +--+--->|---+
 * |           |           |
 * Vs          C           Rl
 * |           |           |
 * +-----------+-----------+
 * 
 * This circuit operates in four distinct modes:
 * 
 * (1) Switch open and diode not conducting. In
 *     this mode the capacitor stores its charge.
 * (2) Switch closed and diode not conducting. In
 *     this mode the capacitor is charged by the
 *     current flowing from the voltage source
 *     through the source resistor.
 * (3) Switch closed and diode conducting. In
 *     mode the capacitor simultaneous charges
 *     from the current coming from the voltage
 *     source and discharges through the load
 *     resistor.
 * (4) Switch open and diode conducting. In this
 *     mode the capacitor discharges through the
 *     load resistor.
 * 
 * The diode is modeled as an ideal device that
 * conducts when the voltage across it is greater
 * than Vd and stops conducting when the voltage
 * drops below Vd. A small hysteris value epsilon
 * is used to prevent the model getting stuck 
 * at Vd.
 */

/**
 * The circuit equations are derived from the ode_system class.
 * The ode_system provides the interface that is used by the
 * Hybrid class to solve the circuit equations numerically.
 */
class Circuit : public adevs::ode_system<> {
  public:
    Circuit()
        /// this model has 1 state variable and 1 state event function
        : adevs::ode_system<>(1, 1),
          switch_conducting(true), // The switch is conducting
          diode_conducting(false),  // The diode is not conducting
          vs(1.0), // The source voltage vs = 1 V
          C(1E-1), // The capacitor has 100 mF
          Rs(1.0), // The source resistor is 1 Ohm
          Rl(10.0), // The load resistor is 10 Ohm
          vd(0.25), // Diode threshold for conducting
          eps(1E-4) {}
    /// Called by the Hybrid class to get the initial values of
    /// the continuous state variables
    void init(double* q) {
        q[0] = 0.0; // Capacitor voltage Vc = 0
    }
    /// Called by the Hybrid class to get the values of the
    /// derivatives of the continuous state variables.
    void der_func(double const* q, double* dq) {
        /// ODE form of the differential equations
        if (!switch_conducting && !diode_conducting) {
            /// dVc/dt = 0 , capacitor holds its charge
            dq[0] = 0.0;
        } else if (!switch_conducting && diode_conducting) {
            /// dVc/dt = -Vc / (C * Rl) , capacitor discharging
            dq[0] = -q[0] / (C * Rl);
        } else if (switch_conducting && !diode_conducting) {
            /// dVc/dt = (Vs - Vc) / (C * Rs) , capacitor charging
            dq[0] = (vs - q[0]) / (C * Rs);
        } else {
            /// dVc/dt = ((Vs - Vc) / Rs - Vc / Rl) / C
            /// Simultaneously charging an discharging
            dq[0] = ((vs - q[0]) / Rs - q[0] / Rl) / C;
        }
    }
    /// Called by the Hybrid class to get the values of the
    /// state event functions. When z = 0 an internal state
    /// transition is triggered. In this example, we monitor
    /// the capacitor voltage to check if it is sufficiently
    /// high for the diode to conduct.
    void state_event_func(double const* q, double* z) {
        if (diode_conducting) {
            /// If the diode is conducting, it stops conducting
            /// when the voltage of the capacitor falls below Vd-eps
            z[0] = q[0] - (vd-eps);
        } else {
            /// If the diode is not conducting, then it starts
            /// conducting when the voltage rises above Vd
            z[0] = q[0] - vd;
        }
    }
    /// As written here, this model does not have any time events.
    double time_event_func(double const*) { return adevs_inf<double>(); }
    /// Internal state transition function of the circuit. Internal events
    /// occur when the state event function produces a value z = 0. In this
    /// model, a state event causes the diode to open or close.
    void internal_event(double*, bool const* events) {
        assert(events[0]);  // only one event type; make sure it fired
        diode_conducting = !diode_conducting;
    }
    /// External state transition function of the circuit. An external event
    /// occurs when the Hybrid model that contains this ode_system receives
    /// input. In this model, an external event changes the state of the
    /// switch.
    void external_event(double*, double, list<adevs::PinValue<>> const &xb) {
        switch_conducting = std::any_cast<bool>(xb.front().value);
    }
    /// Confluent transition function of the circuit.
    void confluent_event(double* q, bool const* events, list<adevs::PinValue<>> const &xb) {
        internal_event(q, events);
        external_event(q, 0.0, xb);
    }
    /// Output function of the circuit. This is called prior to an confluent
    /// or internal event. Place your output in the supplied list. This
    /// output function produces the new state of the diode at a state event.
    void output_func(double const*, bool const* events, list<adevs::PinValue<>> &yb) {
        assert(events[0]);
        yb.push_back(adevs::PinValue<>(diode,!diode_conducting));
    }

    bool getDiode() const { return diode_conducting; }
    bool getSwitch() const { return switch_conducting; }

    const adevs::pin_t diode;

  private:
    bool switch_conducting, diode_conducting;
    double const vs, C, Rs, Rl, vd, eps;
};

/// A switch that opens at time t_open
class OpenSwitch : public adevs::Atomic<> {
  public:
    OpenSwitch(double t_open) : adevs::Atomic<>(), t_open(t_open) {}
    double ta() { return t_open; }
    void delta_int() { t_open = adevs_inf<double>(); }
    void delta_ext(double, list<adevs::PinValue<>> const &) {}
    void delta_conf(list<adevs::PinValue<>> const &) {}
    void output_func(list<adevs::PinValue<>> &yb) {
        yb.push_back(adevs::PinValue<>(open_close,false));
    }

    const adevs::pin_t open_close;

  private:
    double t_open;
};

int main() {
    // Create the model
    auto circuit = new Circuit();
    // The hybrid model adopts the circuit and will delete the circuit when
    // the hybrid is deleted.
    auto hybrid_model = std::make_shared<adevs::ExplicitHybrid<>>(circuit,1E-5,0.01);
    auto open_switch = std::make_shared<OpenSwitch>(0.5);
    auto model = std::make_shared<adevs::Graph<>>();
    model->add_atomic(hybrid_model);
    model->add_atomic(open_switch);
    model->connect(open_switch->open_close, hybrid_model);
    // Create the simulator
    adevs::Simulator<> sim(model);
    // Simulate until the switch and diode have both experienced an event
    double tNow = 0.0;
    while (sim.nextEventTime() < 2.0) {
        cout << tNow << " " << hybrid_model->getState(0) << " "
             << circuit->getSwitch() << " " << circuit->getDiode() << endl;
        tNow = sim.execNextEvent();
    }
    return 0;
}
