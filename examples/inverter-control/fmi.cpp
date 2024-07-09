#include <iostream>
#include "adevs.h"
#include "adevs_corrected_euler.h"
#include "adevs_fmi.h"
#include "adevs_trap.h"

typedef adevs::PortValue<double> IOType;
typedef adevs::Bag<IOType> IOBag;

#include "TestCircuitAdevs.h"
#include "TestCircuitModelica.h"

using namespace std;
using namespace adevs;

// Output interval and maximum step size
double const cint = 1E-4;
// Numerical error tolerance
double const tol = 1E-3;
// Event locator tolerance
double const event_tol = 1E-9;
// Simulation end time
double const tEnd = 1.0;

/**
  * Interface to provide a common mechanism for accessing the
  * variables that we want to print.
  */
class CircuitOutput {
  public:
    CircuitOutput() {}
    // Get the current through the load resistor of the 'a' phase
    virtual double get_i_load_phase_a() = 0;
    virtual double get_inverter_freq(int phase) = 0;
    virtual ~CircuitOutput() {}
};

/// Circuit with modelica inverters.
class TestCircuitModelicaExt : public TestCircuitModelica,
                               public CircuitOutput {
  public:
    TestCircuitModelicaExt() : TestCircuitModelica(), CircuitOutput() {}
    double get_i_load_phase_a() { return get_circuit_a_i_load(); }
    double get_inverter_freq(int phase) {
        if (phase == 0) {
            return get_inverter_1__f();
        }
        if (phase == 1) {
            return get_inverter_2__f();
        }
        if (phase == 2) {
            return get_inverter_3__f();
        }
        return 0.0;
    }
};

/**
  * Create the modelica circuit model for a simulation.
  */
adevs::Devs<IOType>* make_model_with_modelica_inverters(
    CircuitOutput** circuit) {
    // Create our model
    TestCircuitModelicaExt* eqns = new TestCircuitModelicaExt();
    // Wrap a set of solvers around it
    Hybrid<IOType>* model = new Hybrid<IOType>(
        eqns,                                            // Model to simulate
        new trap<IOType>(eqns, tol, cint),               // ODE solver
        new fast_event_locator<IOType>(eqns, event_tol)  // Event locator
        // You must use this event locator for OpenModelica because it does
        // not generate continuous zero crossing functions
    );
    *circuit = eqns;
    return model;
}

/**
  * Use the adevs style of inverter swithing that is external to
  * the FMI.
  */

class Switch : public Atomic<IOType> {
  public:
    Switch(int phase, double freqHz)
        : Atomic<IOType>(), phase(phase), freqHz(freqHz), pos(0), duty(0.0) {
        sched_next();
    }
    void delta_int() {
        // switch from zero to one or vice versa
        pos = next_position();
        sched_next();
    }
    void delta_ext(double e, IOBag const &xb) {
        // Duty cycle is applied at start of each period
        h -= e;
        duty = xb[0].value;
    }
    void delta_conf(IOBag const &xb) {
        pos = next_position();
        // Clip duty cycle so that it is in -1 to 1
        duty = ::max(-1.0, ::min(1.0, xb[0].value));
        sched_next();
    }
    void gc_output(IOBag &) {}
    void output_func(IOBag &yb) {
        IOType y;
        y.port = phase;
        y.value = next_position();
        yb.insert(y);
    }
    double ta() { return h; }

  private:
    int const phase;
    double const freqHz;
    int pos;
    double duty;  // fraction of which pos != 0
    double h;

    int next_position() { return (pos == 0) ? ((duty > 0.0) ? 1 : -1) : 0; }
    void sched_next() {
        h = (pos == 0) ? (1.0 - fabs(duty)) : fabs(duty);
        h /= freqHz;
    }
};

class TestCircuitAdevsExt : public TestCircuitAdevs, public CircuitOutput {
  public:
    TestCircuitAdevsExt() : TestCircuitAdevs(), CircuitOutput() {
        // Zero duty cycle to start of each phase
        duty[0] = duty[1] = duty[2] = 0.0;
    }
    void external_event(double* q, double e, IOBag const &xb) {
        TestCircuitAdevs::external_event(q, e, xb);
        set_switches(xb);
        TestCircuitAdevs::external_event(q, 0.0, xb);
    }
    void confluent_event(double* q, bool const* state_event, IOBag const &xb) {
        TestCircuitAdevs::confluent_event(q, state_event, xb);
        set_switches(xb);
        TestCircuitAdevs::confluent_event(q, state_event, xb);
    }
    void output_func(double const* q, bool const* state_event, IOBag &yb) {
        IOType y;
        double duty_cycle;
        TestCircuitAdevs::output_func(q, state_event, yb);
        // If the duty cycle has changed, set the new value
        // to the switch models
        duty_cycle = get_inverter_1__duty_cycle_for_de();
        if (duty_cycle != duty[0]) {
            y.port = 0;
            y.value = duty_cycle;
            yb.insert(y);
            duty[0] = this->get_inverter_1__duty_cycle_for_de();
        }
        duty_cycle = get_inverter_2__duty_cycle_for_de();
        if (duty_cycle != duty[1]) {
            y.port = 1;
            y.value = duty_cycle;
            yb.insert(y);
            duty[1] = this->get_inverter_2__duty_cycle_for_de();
        }
        duty_cycle = get_inverter_3__duty_cycle_for_de();
        if (duty_cycle != duty[2]) {
            y.port = 2;
            y.value = duty_cycle;
            yb.insert(y);
            duty[2] = this->get_inverter_3__duty_cycle_for_de();
        }
    }
    double get_i_load_phase_a() { return get_circuit_a_i_load(); }
    double get_inverter_freq(int phase) {
        if (phase == 0) {
            return get_inverter_1__f();
        }
        if (phase == 1) {
            return get_inverter_2__f();
        }
        if (phase == 2) {
            return get_inverter_3__f();
        }
        return 0.0;
    }

  private:
    double duty[3];

    void set_switches(IOBag const &xb) {
        for (auto x : xb) {
            switch (x.port) {
                case 0:
                    set_adevs_switch_1_(x.value);
                    break;
                case 1:
                    set_adevs_switch_2_(x.value);
                    break;
                case 2:
                    set_adevs_switch_3_(x.value);
                    break;
                default:
                    break;
            }
        }
    }
};

adevs::Devs<IOType>* make_model_with_adevs_inverters(CircuitOutput** circuit) {
    // Create our model
    Switch* inverter_switch[3];
    TestCircuitAdevsExt* eqns = new TestCircuitAdevsExt();
    // Wrap a set of solvers around it. This also initializes the FMI
    // so that we can get its variables.
    Hybrid<IOType>* model = new Hybrid<IOType>(
        eqns,                                     // Model to simulate
        new trap<IOType>(eqns, tol, cint, true),  // ODE solver
        new fast_event_locator<IOType>(eqns, event_tol, true)  // Event locator
        // You must use this event locator for OpenModelica because it does
        // not generate continuous zero crossing functions
    );
    // Create switches and connect everything
    Digraph<double>* dig = new Digraph<double>();
    dig->add(model);
    for (int i = 0; i < 3; i++) {
        inverter_switch[i] = new Switch(i, eqns->get_inverter_freq(i));
        dig->add(inverter_switch[i]);
        dig->couple(inverter_switch[i], i, model, i);
        dig->couple(model, i, inverter_switch[i], i);
    }
    *circuit = eqns;
    return dig;
}

int main() {
    CircuitOutput* circuit;
    /* Pick one or the other! */
    //adevs::Devs<IOType>* model = make_model_with_adevs_inverters(&circuit);
    adevs::Devs<IOType>* model = make_model_with_modelica_inverters(&circuit);

    // Create the simulator
    Simulator<IOType>* sim = new Simulator<IOType>(model);
    // Run the simulation
    double tL = 0.0;
    while (sim->nextEventTime() < tEnd) {
        bool print = sim->nextEventTime() > tL + cint;
        if (print) {
            tL = sim->nextEventTime();
            cout << tL;
        }
        sim->execNextEvent();
        if (print) {
            cout << "," << circuit->get_i_load_phase_a() << endl;
        }
    }
    // Cleanup
    delete sim;
    delete model;
    // Done!
    return 0;
}
