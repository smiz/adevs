#include <iostream>
#include <typeinfo>
#include "adevs/adevs.h"
#include "check_ball1d_solution.h"
#include "sampler.h"
using namespace std;
using namespace adevs;

/**
 * Solves
 * 		dx/dt = -y
 * 		y = (x-y)/2
 */
class dae : public dae_se1_system<double> {
  public:
    dae() : dae_se1_system<double>(2, 1, 1) {}
    void init(double* q, double* a) {
        q[1] = 0.0;
        q[0] = 1.0;
        a[0] = q[0] / 3.0;
    }
    void alg_func(double const* q, double const* a, double* af) {
        af[0] = 0.5 * (q[0] - a[0]);
    }
    void der_func(double const* q, double const* a, double* dq) {
        dq[1] = 1.0;
        dq[0] = -a[0];
    }
    void state_event_func(double const* q, double const* a, double* z) {
        z[0] = 1.0;
    }
    double time_event_func(double const* q, double const* a) { return DBL_MAX; }
    void internal_event(double* q, double* a, bool const* event_flag) {
        assert(false);
    }
    void postStep(double* q, double* a) { check_soln(q, a); }
    void external_event(double* q, double* a, double e, Bag<double> const &xb) {
        check_soln(q, a);
    }
    void confluent_event(double* q, double* a, bool const* event_flag,
                         Bag<double> const &xb) {
        assert(false);
    }
    void output_func(double const* q, double const* a, bool const* event_flag,
                     Bag<double> &yb) {
        assert(false);
    }


  private:
    void check_soln(double const* q, double const* a) {
        double af[1];
        alg_func(q, a, af);
        assert(fabs(af[0] - a[0]) < 1E-5);
        assert(fabs(q[0] - exp(-q[1] / 3.0)) < 1E-5);
    }
};

class SolutionPlotter : public EventListener<double> {
  public:
    SolutionPlotter(Hybrid<double>* hsys, dae_se1_system<double>* dsys)
        : hsys(hsys), dsys(dsys) {}
    void stateChange(Atomic<double>* model, double t) {
        cout << t << " " << hsys->getState(0) << " " << dsys->getAlgVar(0)
             << endl;
    }
    void outputEvent(Event<double> y, double t) {}

  private:
    Hybrid<double>* hsys;
    dae_se1_system<double>* dsys;
};

void run_test(dae_se1_system<double>* b, ode_solver<double>* s,
              event_locator<double>* l) {
    Hybrid<double>* model = new Hybrid<double>(b, s, l);
    SolutionPlotter* plotter = new SolutionPlotter(model, b);
    Simulator<double>* sim = new Simulator<double>(model);
    sim->addEventListener(plotter);
    while (sim->nextEventTime() < 1.0) {
        sim->execNextEvent();
    }
    delete sim;
    delete model;
    delete plotter;
}

int main() {
    dae_se1_system<double>* sys = new dae();
    run_test(sys, new rk_45<double>(sys, 1E-6, 0.01),
             new linear_event_locator<double>(sys, 1E-7));
    return 0;
}
