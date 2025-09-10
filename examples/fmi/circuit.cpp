#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h" // Get the ModelExchange class
#include "adevs/solvers/trap.h" // Get the ImplicitHybrid class

using PinValue = adevs::PinValue<>;

/**
 * This is exactly the same as tutorial/ex5.cpp and Example #5
 * in the main page of the documention. The only difference
 * is that the analog circuit is modeling using OpenModelica
 * and we import it into the discrete event simulation as an
 * FMI for ModelExchange.
 * 
 * You can build the fmu and executable with
 * 
 * make
 * 
 * You will need to have the OpenModelica compiler and SUNDIALS
 * installed for this example to run.
 */

/**
 * The circuit equations are derived from the ModelExchange class.
 * The ModelExchange class in turn implements most of the methods
 * of the ode_system class by calling on the FMI functions provided
 * by the fmu bundle that we pass to the constructor.
 */
class Circuit : public adevs::ModelExchange<> {
  public:
    Circuit() : ModelExchange("Circuit.fmu",1E-6) {}
    /// The external state transition function sets the state of
    /// the switch.
    void external_event(double* state, double e, std::list<PinValue> const &xb) {
        ModelExchange::external_event(state,0.0,xb);
        set_variable("switch",1);
        /// Process the change in state of the switch
        ModelExchange::external_event(state,0.0,xb);
    }
    /// Confluent transition function of the circuit.
    void confluent_event(double* state, bool const* events, std::list<PinValue> const &xb) {
        ModelExchange::confluent_event(state,events,xb);
        set_variable("switch",1);
        /// Process the change in state of the switch
        ModelExchange::confluent_event(state,events,xb);
    }
    /// Output function of the circuit. This is called prior to an confluent
    /// or internal event. Place your output in the supplied list. This
    /// output function produces the new state of the diode at a state event.
    void output_func(double const*, bool const*, std::list<PinValue> &yb) {
        yb.push_back(PinValue(diode,get_variable("D.off")));
    }

    bool getDiode() { return std::any_cast<int>(get_variable("D.off")); }
    bool getSwitch() { return std::any_cast<int>(get_variable("switch")); }

    const adevs::pin_t diode;
};

/// A switch that opens at time t_open
class OpenSwitch : public adevs::Atomic<> {
  public:
    OpenSwitch(double t_open) : adevs::Atomic<>(), t_open(t_open) {}
    double ta() { return t_open; }
    void delta_int() { t_open = adevs_inf<double>(); }
    void delta_ext(double, std::list<adevs::PinValue<>> const &) {}
    void delta_conf(std::list<PinValue> const &) {}
    void output_func(std::list<PinValue> &yb) {
        yb.push_back(PinValue(open_close,false));
    }

    const adevs::pin_t open_close;

  private:
    double t_open;
};

int main() {
    // Create the model
    auto circuit = new Circuit();
    // The hybrid model adopts the circuit and will delete the circuit when
    // the hybrid is deleted. We use the implicit solver here because the
    // Modelica model includes some parasitic capacitances that are very small
    auto hybrid_model = std::make_shared<adevs::ImplicitHybrid<>>(circuit,1E-5,0.01);
    auto open_switch = std::make_shared<OpenSwitch>(0.5);
    auto model = std::make_shared<adevs::Graph<>>();
    model->add_atomic(hybrid_model);
    model->add_atomic(open_switch);
    model->connect(open_switch->open_close, hybrid_model);
    // Create the simulator
    adevs::Simulator<> sim(model);
    // Simulate until the switch and diode have both experienced an event
    double tNow = 0.0;
    while (sim.nextEventTime() < 5.0) {
        std::cout << tNow << " " << hybrid_model->getState(0) << " "
             << circuit->getSwitch() << " " << circuit->getDiode() << std::endl;
        tNow = sim.execNextEvent();
    }
    return 0;
}
