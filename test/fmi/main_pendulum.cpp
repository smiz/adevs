#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"


using ModelExchange = adevs::ModelExchange;
using Hybrid = adevs::Hybrid;
using PinValue = adevs::PinValue;
using Graph = adevs::Graph;
using pin_t = adevs::pin_t;
using ode_system = adevs::ode_system<double>;

class oracle : public ode_system {
  public:
    oracle() : ode_system(3, 0), test_count(0) {}
    void init(double* q) {
        q[0] = 0.0;
        q[1] = 0.0;
        q[2] = 0.0;
    }
    void der_func(double const* q, double* dq) {
        dq[0] = q[1];
        dq[1] = -cos(q[0]) * 9.82;
        dq[2] = -1.0;
    }
    void state_event_func(double const*, double*) {}
    double time_event_func(double const* q) { return q[2]; }
    void internal_event(double* q, bool const*) { q[2] = 0.01; }
    void external_event(double* q, double, std::list<PinValue> const &xb) {
        static double const pi = 3.1415926535897931;
        test_count++;
        double test_angle = xb.front().value;
        double diff = fabs(q[0] - test_angle);
        if (diff < -1E-3) {
            diff += 2.0 * pi;
        }
        if (diff > 1E-3) {
            diff -= 2.0 * pi;
        }
        if (fabs(diff) > 1E-3) {
            std::cerr << "AGGGH: " << q[0] << "," << test_angle << "," << diff << std::endl;
        }
        assert(fabs(diff) < 1E-3);
    }
    void confluent_event(double*, bool const*, std::list<PinValue> const &) { assert(false); }
    void output_func(double const*, bool const*, std::list<PinValue> &yb) {
        yb.push_back(PinValue(output, 0));
    }
    int getTestCount() { return test_count; }

    pin_t const output;

  private:
    int test_count;
};

class pendulum : public ModelExchange {
  public:
    pendulum() : ModelExchange("pendulum.fmu", 1E-6), query(false) {}
    double time_event_func(double const* q) {
        ModelExchange::time_event_func(q);
        if (query) {
            return 0;
        } else {
            return adevs_inf<double>();
        }
    }
    void internal_event(double* q, bool const* state_events) {
        ModelExchange::internal_event(q, state_events);
        query = false;
    }
    void external_event(double* q, double e, std::list<PinValue> const &xb) {
        ModelExchange::external_event(q, e, xb);
        query = true;
    }
    void output_func(double const* q, bool const* state_events, std::list<PinValue> &yb) {
        double theta = std::any_cast<double>(get_variable("theta"));
        ModelExchange::output_func(q, state_events, yb);
        yb.push_back(PinValue(output, theta));
    }

    pin_t const output;

  private:
    bool query;
};

int main() {
    // Create the open modelica model
    pendulum* model = new pendulum();
    std::shared_ptr<Hybrid> hybrid_model =
        std::make_shared<Hybrid>(model, new corrected_euler<double>(model, 1E-8, 0.01),
                                 new discontinuous_event_locator<double>(model, 1E-6));
    // Create the test oracle
    oracle* test_oracle = new oracle();
    std::shared_ptr<Hybrid> hybrid_model_oracle =
        std::make_shared<Hybrid>(test_oracle, new corrected_euler<double>(test_oracle, 1E-8, 0.01),
                                 new linear_event_locator<double>(test_oracle, 1E-6));
    // Combine them
    std::shared_ptr<Graph> dig_model = std::make_shared<Graph>();
    dig_model->add_atomic(hybrid_model);
    dig_model->add_atomic(hybrid_model_oracle);
    dig_model->connect(model->output, hybrid_model_oracle);
    dig_model->connect(test_oracle->output, hybrid_model);
    // Create the simulator
    Simulator* sim = new Simulator(dig_model);
    double theta = std::any_cast<double>(model->get_variable("theta"));
    assert(fabs(theta) < 1E-6);
    std::cout << "# time, x, y" << std::endl;
    while (sim->nextEventTime() <= 15.0) {
        std::cout << sim->nextEventTime() << " ";
        sim->execNextEvent();
        theta = std::any_cast<double>(model->get_variable("theta"));
        double x = std::any_cast<double>(model->get_variable("x"));
        double y = std::any_cast<double>(model->get_variable("y"));
        std::cout << x << " " << y << " " << theta << " " << hybrid_model_oracle->getState(0) << " "
                  << std::endl;
    }
    assert(test_oracle->getTestCount() > 0);
    delete sim;
    return 0;
}
