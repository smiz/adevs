#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"
#include "pendulum/modelDescription.h"
using namespace std;
using namespace adevs;

class oracle : public ode_system<double> {
  public:
    oracle() : ode_system<double>(3, 0), test_count(0) {}
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
    void external_event(double* q, double, list<PinValue<double>> const &xb) {
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
            cerr << "AGGGH: " << q[0] << "," << test_angle << "," << diff
                 << endl;
        }
        assert(fabs(diff) < 1E-3);
    }
    void confluent_event(double*, bool const*, list<PinValue<double>> const &) {
        assert(false);
    }
    void output_func(double const*, bool const*, list<PinValue<double>> &yb) {
        yb.push_back(PinValue<double>(output,0));
    }
    int getTestCount() { return test_count; }

    const adevs::pin_t output;

  private:
    int test_count;
};

class pendulum2 : public pendulum {
  public:
    pendulum2() : pendulum(), query(false) {}
    double time_event_func(double const* q) {
        pendulum::time_event_func(q);
        if (query) {
            return 0;
        } else {
            return adevs_inf<double>();
        }
    }
    void internal_event(double* q, bool const* state_events) {
        pendulum::internal_event(q, state_events);
        query = false;
    }
    void external_event(double* q, double e, list<PinValue<double>> const &xb) {
        pendulum::external_event(q, e, xb);
        query = true;
    }
    void output_func(double const* q, bool const* state_events,
                     list<PinValue<double>> &yb) {
        pendulum::output_func(q, state_events, yb);
        yb.push_back(PinValue<double>(output,get_theta()));
    }

    const pin_t output;

  private:
    bool query;
};

int main() {
    // Create the open modelica model
    pendulum2* model = new pendulum2();
    shared_ptr<Hybrid<double>> hybrid_model = make_shared<Hybrid<double>>(
        model, new corrected_euler<double>(model, 1E-8, 0.01),
        new discontinuous_event_locator<double>(model, 1E-6));
    // Create the test oracle
    oracle* test_oracle = new oracle();
    shared_ptr<Hybrid<double>> hybrid_model_oracle = make_shared<Hybrid<double>>(
        test_oracle, new corrected_euler<double>(test_oracle, 1E-8, 0.01),
        new linear_event_locator<double>(test_oracle, 1E-6));
    // Combine them
    shared_ptr<Graph<double>> dig_model = make_shared<Graph<double>>();
    dig_model->add_atomic(hybrid_model);
    dig_model->add_atomic(hybrid_model_oracle);
    dig_model->connect(model->output, hybrid_model_oracle);
    dig_model->connect(test_oracle->output, hybrid_model);
    // Create the simulator
    Simulator<double>* sim = new Simulator<double>(dig_model);
    assert(fabs(model->get_theta()) < 1E-6);
    cout << "# time, x, y" << endl;
    while (sim->nextEventTime() <= 15.0) {
        cout << sim->nextEventTime() << " ";
        sim->execNextEvent();
        cout << model->get_x() << " " << model->get_y() << " "
             << model->get_theta() << " " << hybrid_model_oracle->getState(0)
             << " " << endl;
    }
    assert(test_oracle->getTestCount() > 0);
    delete sim;
    return 0;
}
