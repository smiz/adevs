#include <iostream>
#include <typeinfo>
#include "adevs/adevs.h"
#include "check_ball1d_solution.h"
#include "sampler.h"

// using namespace adevs;

const adevs::pin_t ball_output;

/**
 * Simple test which simulates a bouncing ball. The output value is the height of
 * the ball. Input values cause the system to produce an output sample immediately.
 */
class bouncing_ball : public adevs::ode_system<double> {
  public:
    bouncing_ball()
        : adevs::ode_system<double>(3, 1), sample(false), phase(FALL) {}
    void init(double* q) {
        last_event_time = 0.0;
        q[0] = 1.0;  // Initial height
        q[1] = 0.0;  // Initial velocity
        q[2] = 0.0;  // Time
    }
    void der_func(double const* q, double* dq) {
        dq[0] = q[1];
        dq[1] = -2.0;  // For test case
        dq[2] = 1.0;
    }
    void state_event_func(double const* q, double* z) {
        if (phase == FALL) {
            z[0] = q[0];  // Bounce if it is going down
        } else {
            z[0] = q[1];  // Start falling at apogee
        }
    }
    double time_event_func(double const*) {
        if (sample) {
            return 0.0;
        } else {
            return DBL_MAX;
        }
    }
    void internal_event(double* q, bool const* event_flag) {
        if (event_flag[0]) {
            if (phase == FALL)  // Hit the ground
            {
                phase = CLIMB;
                q[1] = -q[1];
            } else  // reach apogee
            {
                phase = FALL;
            }
        }
        sample = false;
        last_event_time = q[2];
    }
    void external_event(double* q, double e,
                        std::list<adevs::PinValue<double>> const &xb) {
        assert(fabs(q[2] - last_event_time - e) < 1E-9);
        sample = xb.size() > 0;
        last_event_time = q[2];
    }
    void confluent_event(double* q, bool const* event_flag,
                         std::list<adevs::PinValue<double>> const &xb) {
        internal_event(q, event_flag);
        external_event(q, 0.0, xb);
    }
    void output_func(double const* q, bool const* event_flag,
                     std::list<adevs::PinValue<double>> &yb) {
        assert(event_flag[0] || event_flag[1]);
        adevs::PinValue<double> event(ball_output, q[0]);
        yb.push_back(event);
    }

  private:
    bool sample;
    double last_event_time;
    enum { CLIMB = 0, FALL = 1 } phase;
};

class SolutionChecker : public adevs::EventListener<double> {
  public:
    SolutionChecker(std::shared_ptr<adevs::Hybrid<double>> ball) : ball(ball) {}
    void stateChange(adevs::Atomic<double>& model, double t) {
        if (&model == ball.get()) {
            assert(ball1d_soln_ok(t, ball->getState(0)));
        }
    }
    void outputEvent(adevs::Atomic<double>&, adevs::PinValue<double>&, double){}
    void inputEvent(adevs::Atomic<double>&, adevs::PinValue<double>&, double){}
  private:
    std::shared_ptr<adevs::Hybrid<double>> ball;
};

void run_test(adevs::ode_system<double>* b,
			  adevs::ode_solver<double>* s,
			  adevs::event_locator<double>* l) {
    std::cerr << "Testing " << typeid(*s).name() << " , " << typeid(*l).name()
         << std::endl;
    std::shared_ptr<adevs::Hybrid<double>> ball = std::make_shared<adevs::Hybrid<double>>(b, s, l);
    std::shared_ptr<sampler> sample = std::make_shared<sampler>(0.01);
    std::shared_ptr<adevs::Graph<double>> model = std::make_shared<adevs::Graph<double>>();
    model->add_atomic(ball);
    model->add_atomic(sample);
    model->connect(sample->sample_pin, ball);
    model->connect(ball_output, sample);
    std::shared_ptr<SolutionChecker> checker = std::make_shared<SolutionChecker>(ball);
    adevs::Simulator sim(model);
    sim.addEventListener(checker);
    while (sim.nextEventTime() < 10.0) {
        sim.execNextEvent();
    }
}

int main() {
    // Test fast algorithm without interpolation
    bouncing_ball* ball = new bouncing_ball();
    run_test(ball, new adevs::corrected_euler<double>(ball, 1E-6, 0.01),
             new adevs::fast_event_locator<double>(ball, 1E-7));
    // Test fast algorithm with interpolation
    ball = new bouncing_ball();
    run_test(ball, new adevs::corrected_euler<double>(ball, 1E-6, 0.01),
             new adevs::fast_event_locator<double>(ball, 1E-7, true));
    // Test linear algorithm
    ball = new bouncing_ball();
    run_test(ball, new adevs::corrected_euler<double>(ball, 1E-6, 0.01),
             new adevs::linear_event_locator<double>(ball, 1E-7));
    // Test RK 45
    ball = new bouncing_ball();
    run_test(ball, new adevs::rk_45<double>(ball, 1E-6, 0.01),
             new adevs::linear_event_locator<double>(ball, 1E-7));
    // Test bisection algorithm
    ball = new bouncing_ball();
    run_test(ball, new adevs::corrected_euler<double>(ball, 1E-6, 0.01),
             new adevs::bisection_event_locator<double>(ball, 1E-7));
    ball = new bouncing_ball();
    run_test(ball, new adevs::rk_45<double>(ball, 1E-6, 0.01),
             new adevs::bisection_event_locator<double>(ball, 1E-7));
    return 0;
}
