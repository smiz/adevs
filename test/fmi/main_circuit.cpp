#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"

using namespace adevs;

class Circuit : public ModelExchange<double> {
  public:
    Circuit() : ModelExchange<double>("Circuit.fmu",1E-7), start_time(adevs_inf<double>()) {}
    void external_event(double* q, double e, std::list<PinValue<double>> const &xb) {
        ModelExchange<double>::external_event(q, e, xb);
        start_time = e;
        ModelExchange<double>::set_variable("Vsrc.Vref",0.0);
        ModelExchange<double>::external_event(q, e, xb);
    }
    void print_state() {
        double Vsrc_T_v = std::any_cast<double>(get_variable("Vsrc.T.v"));
        std::cout << get_time() << " " << Vsrc_T_v << " " << std::endl;
    }
    void test_state() {
        double Vsrc_T_v = std::any_cast<double>(get_variable("Vsrc.T.v"));
        double R2_T2_v = std::any_cast<double>(get_variable("R2.T2.v"));
        double R1_T2_v = std::any_cast<double>(get_variable("R1.T2.v"));
        double Rbridge_T1_i = std::any_cast<double>(get_variable("Rbridge.T1.i"));
        double v = 1.0;
        if (get_time() > start_time) {
            v = exp(start_time - get_time());
            assert(fabs(v - Vsrc_T_v) < 1E-3);
        } else {
            assert(fabs(v - Vsrc_T_v) < 1E-6);
        }
        assert(fabs(R2_T2_v - v / 2.0) < 1E-6);
        assert(fabs(R1_T2_v - v / 2.0) < 1E-6);
        assert(fabs(Rbridge_T1_i) < 1E-6);
    }

    const pin_t input;

  private:
    double start_time;
};

int main() {
    Circuit* test_model = new Circuit();
    std::shared_ptr<Hybrid<double>> hybrid_model = std::make_shared<Hybrid<double>>(
        test_model, new corrected_euler<double>(test_model, 1E-7, 0.001),
        new discontinuous_event_locator<double>(test_model, 1E-7));
    std::shared_ptr<Graph<double>> graph = std::make_shared<Graph<double>>();
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
