#include <iostream>
#include "adevs/adevs.h"

// using namespace adevs;

static double const period = 0.001;

// Produces an event every period units of time
class Genr : public adevs::Atomic<int> {
  public:
    Genr() : adevs::Atomic<int>() {}
    double ta() { return period; }
    void delta_int() {}
    void delta_ext(double, std::list<adevs::PinValue<int>> const &) {}
    void delta_conf(std::list<adevs::PinValue<int>> const &) {}
    void output_func(std::list<adevs::PinValue<int>> &yb) { yb.push_back(adevs::PinValue<int>(output,1)); }

    const adevs::pin_t output;
};

// Also undergoes an internal transition every
// period units of time. All state transitions
// should be confluent transitions.
class test_model : public adevs::ode_system<int> {
  public:
    test_model() : adevs::ode_system<int>(1, 0), is_confluent(false) {}
    void init(double* q) { q[0] = period; }
    void der_func(double const*, double* dq) { dq[0] = -1.0; }
    void state_event_func(double const*, double*) {}
    double time_event_func(double const* q) { return q[0]; }
    void internal_event(double* q, bool const*) {
        assert(is_confluent);
        q[0] = period;
    }
    void external_event(double*, double, std::list<adevs::PinValue<int>> const &xb) {
        assert(is_confluent);
        assert(xb.size() == 1);
    }
    void confluent_event(double* q, bool const* event_flag,
                         std::list<adevs::PinValue<int>> const &xb) {
        is_confluent = true;
        internal_event(q, event_flag);
        external_event(q, 0.0, xb);
        is_confluent = false;
    }
    void output_func(double const*, bool const*, std::list<adevs::PinValue<int>> &) {}


  private:
    bool is_confluent;
};

void run_test(adevs::ode_system<int>* b, adevs::ode_solver<int>* s, adevs::event_locator<int>* l) {
    std::shared_ptr<adevs::Hybrid<int>> ball = std::make_shared<adevs::Hybrid<int>>(b, s, l);
    std::shared_ptr<Genr> genr = std::make_shared<Genr>();
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    model->add_atomic(ball);
    model->add_atomic(genr);
    model->connect(genr->output, ball);
    adevs::Simulator<int>* sim = new adevs::Simulator<int>(model);
    while (sim->nextEventTime() < 10.0) {
        sim->execNextEvent();
    }
    delete sim;
}

int main() {
    // Test linear algorithm
    test_model* ball = new test_model();
    run_test(ball, new adevs::corrected_euler<int>(ball, 1E-6, 0.01),
             new adevs::linear_event_locator<int>(ball, 1E-7));
    return 0;
}
