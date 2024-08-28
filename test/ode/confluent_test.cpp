#include <iostream>
#include "adevs/adevs.h"
using namespace std;
using namespace adevs;

static double const period = 0.001;

// Produces an event every period units of time
class Genr : public Atomic<int> {
  public:
    Genr() : Atomic<int>() {}
    double ta() { return period; }
    void delta_int() {}
    void delta_ext(double, list<int> const &) {}
    void delta_conf(list<int> const &) {}
    void output_func(list<int> &yb) { yb.push_back(1); }
};

// Also undergoes an internal transition every
// period units of time. All state transitions
// should be confluent transitions.
class test_model : public ode_system<int> {
  public:
    test_model() : ode_system<int>(1, 0), is_confluent(false) {}
    void init(double* q) { q[0] = period; }
    void der_func(double const* q, double* dq) { dq[0] = -1.0; }
    void state_event_func(double const* q, double* z) {}
    double time_event_func(double const* q) { return q[0]; }
    void internal_event(double* q, bool const* event_flag) {
        assert(is_confluent);
        q[0] = period;
    }
    void external_event(double* q, double e, list<int> const &xb) {
        assert(is_confluent);
        assert(xb.size() == 1);
    }
    void confluent_event(double* q, bool const* event_flag,
                         list<int> const &xb) {
        is_confluent = true;
        internal_event(q, event_flag);
        external_event(q, 0.0, xb);
        is_confluent = false;
    }
    void output_func(double const*, bool const*, list<int> &) {}


  private:
    bool is_confluent;
};

void run_test(ode_system<int>* b, ode_solver<int>* s, event_locator<int>* l) {
    Hybrid<int>* ball = new Hybrid<int>(b, s, l);
    Genr* genr = new Genr();
    SimpleDigraph<int>* model = new SimpleDigraph<int>();
    model->add(ball);
    model->add(genr);
    model->couple(genr, ball);
    Simulator<int>* sim = new Simulator<int>(model);
    while (sim->nextEventTime() < 10.0) {
        sim->execNextEvent();
    }
    delete sim;
    delete model;
}

int main() {
    // Test linear algorithm
    test_model* ball = new test_model();
    run_test(ball, new corrected_euler<int>(ball, 1E-6, 0.01),
             new linear_event_locator<int>(ball, 1E-7));
    return 0;
}
