#include <iostream>
#include "adevs/adevs.h"
#include "adevs/fmi.h"
#include "circuit/modelDescription.h"
using namespace std;
using namespace adevs;

class Circuit2 : public Circuit {
  public:
    Circuit2() : Circuit(), start_time(DBL_MAX) {}
    void external_event(double* q, double e, list<double> const &xb) {
        Circuit::external_event(q, e, xb);
        start_time = e;
        set_Vsrc_Vref(0.0);
        Circuit::external_event(q, e, xb);
    }
    void print_state() {
        cout << get_time() << " " << get_Vsrc_T_v() << " " << endl;
    }
    void test_state() {
        double v = 1.0;
        if (get_time() > start_time) {
            v = exp(start_time - get_time());
            assert(fabs(v - get_Vsrc_T_v()) < 1E-3);
        } else {
            assert(fabs(v - get_Vsrc_T_v()) < 1E-6);
        }
        assert(fabs(get_R2_T2_v() - v / 2.0) < 1E-6);
        assert(fabs(get_R1_T2_v() - v / 2.0) < 1E-6);
        assert(fabs(get_Rbridge_T1_i()) < 1E-6);
    }

  private:
    double start_time;
};

int main() {
    Circuit2* test_model = new Circuit2();
    Hybrid<double>* hybrid_model = new Hybrid<double>(
        test_model, new corrected_euler<double>(test_model, 1E-7, 0.001),
        new discontinuous_event_locator<double>(test_model, 1E-7));
    // Create the simulator
    Simulator<double>* sim = new Simulator<double>(hybrid_model);
    // Check initial values
    test_model->print_state();
    // Run the simulation, testing the solution as we go
    while (sim->nextEventTime() <= 1.0) {
        sim->execNextEvent();
        test_model->print_state();
        test_model->test_state();
    }
    list<Event<double>> xb;
    Event<double> event(hybrid_model, 0.0);
    xb.push_back(event);
    sim->computeNextState(xb, 1.0);
    while (sim->nextEventTime() <= 5.0) {
        sim->execNextEvent();
        test_model->print_state();
        test_model->test_state();
    }
    delete sim;
    delete hybrid_model;
    return 0;
}
