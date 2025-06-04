#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"
#include "circuit/modelDescription.h"
using namespace std;
using namespace adevs;

class Circuit2 : public Circuit {
  public:
    Circuit2() : Circuit(), start_time(adevs_inf<double>()) {}
    void external_event(double* q, double e, list<PinValue<double>> const &xb) {
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

    const pin_t input;

  private:
    double start_time;
};

int main() {
    Circuit2* test_model = new Circuit2();
    shared_ptr<Hybrid<double>> hybrid_model = make_shared<Hybrid<double>>(
        test_model, new corrected_euler<double>(test_model, 1E-7, 0.001),
        new discontinuous_event_locator<double>(test_model, 1E-7));
    shared_ptr<Graph<double>> graph = make_shared<Graph<double>>();
    graph->add_atomic(hybrid_model);
    graph->connect(test_model->input,hybrid_model);
    // Create the simulator
    Simulator<double>* sim = new Simulator<double>(graph);
    // Check initial values
    test_model->print_state();
    // Run the simulation, testing the solution as we go
    while (sim->nextEventTime() <= 1.0) {
        sim->execNextEvent();
        test_model->print_state();
        test_model->test_state();
    }
    PinValue<double> inject(test_model->input,0.0);
    sim->setNextTime(1.0);
    sim->injectInput(inject);
    while (sim->nextEventTime() <= 5.0) {
        sim->execNextEvent();
        test_model->print_state();
        test_model->test_state();
    }
    delete sim;
    return 0;
}
