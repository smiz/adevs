#include <iostream>
#include "adevs/adevs.h"


using pin_t = adevs::pin_t;
using Simulator = adevs::Simulator<int>;
using Atomic = adevs::Atomic<int>;
using Hybrid = adevs::Hybrid<int>;
using Graph = adevs::Graph<int>;
using corrected_euler = adevs::corrected_euler<int>;
using PinValue = adevs::PinValue<int>;
using ode_system = adevs::ode_system<int>;
using ode_solver = adevs::ode_solver<int>;
using event_locator = adevs::event_locator<int>;
using linear_event_locator = adevs::linear_event_locator<int>;

static double const period = 0.001;

// Produces an event every period units of time
class Genr : public Atomic {
  public:
    Genr() : Atomic() {}
    double ta() { return period; }
    void delta_int() {}
    void delta_ext(double, std::list<PinValue> const &) {}
    void delta_conf(std::list<PinValue> const &) {}
    void output_func(std::list<PinValue> &yb) { yb.push_back(PinValue(output, 1)); }

    pin_t const output;
};

// Also undergoes an internal transition every
// period units of time. All state transitions
// should be confluent transitions.
class test_model : public ode_system {
  public:
    test_model() : ode_system(1, 0), is_confluent(false) {}
    void init(double* q) { q[0] = period; }
    void der_func(double const*, double* dq) { dq[0] = -1.0; }
    void state_event_func(double const*, double*) {}
    double time_event_func(double const* q) { return q[0]; }
    void internal_event(double* q, bool const*) {
        assert(is_confluent);
        q[0] = period;
    }
    void external_event(double*, double, std::list<PinValue> const &xb) {
        assert(is_confluent);
        assert(xb.size() == 1);
    }
    void confluent_event(double* q, bool const* event_flag, std::list<PinValue> const &xb) {
        is_confluent = true;
        internal_event(q, event_flag);
        external_event(q, 0.0, xb);
        is_confluent = false;
    }
    void output_func(double const*, bool const*, std::list<PinValue> &) {}


  private:
    bool is_confluent;
};

void run_test(ode_system* b, ode_solver* s, event_locator* l) {
    std::shared_ptr<Hybrid> ball = std::make_shared<Hybrid>(b, s, l);
    std::shared_ptr<Genr> genr = std::make_shared<Genr>();
    std::shared_ptr<Graph> model = std::make_shared<Graph>();
    model->add_atomic(ball);
    model->add_atomic(genr);
    model->connect(genr->output, ball);
    Simulator* sim = new Simulator(model);
    while (sim->nextEventTime() < 10.0) {
        sim->execNextEvent();
    }
    delete sim;
}

int main() {
    // Test linear algorithm
    test_model* ball = new test_model();
    run_test(ball, new corrected_euler(ball, 1E-6, 0.01), new linear_event_locator(ball, 1E-7));
    return 0;
}
