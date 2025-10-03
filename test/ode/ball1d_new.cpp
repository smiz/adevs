#include <iostream>
#include <typeinfo>
#include "adevs/adevs.h"
#include "check_ball1d_solution.h"
#include "sampler.h"


using Simulator = adevs::Simulator<double>;
using Hybrid = adevs::Hybrid<double>;
using Graph = adevs::Graph<double>;
using PinValue = adevs::PinValue<double>;
using Atomic = adevs::Atomic<double>;
using EventListener = adevs::EventListener<double>;
using pin_t = adevs::pin_t;
using corrected_euler = adevs::corrected_euler<double>;
using ode_system = adevs::ode_system<double>;
using ode_solver = adevs::ode_solver<double>;
using event_locator = adevs::event_locator<double>;
using fast_event_locator = adevs::fast_event_locator<double>;
using bisection_event_locator = adevs::bisection_event_locator<double>;
using linear_event_locator = adevs::linear_event_locator<double>;
using rk_45 = adevs::rk_45<double>;

pin_t const ball_output;

/**
 * Simple test which simulates a bouncing ball. The output value is the height of
 * the ball. Input values cause the system to produce an output sample immediately.
 */
class bouncing_ball : public ode_system {
  public:
    bouncing_ball() : ode_system(3, 1), sample(false), phase(FALL) {}
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
    void external_event(double* q, double e, std::list<PinValue> const &xb) {
        assert(fabs(q[2] - last_event_time - e) < 1E-9);
        sample = xb.size() > 0;
        last_event_time = q[2];
    }
    void confluent_event(double* q, bool const* event_flag, std::list<PinValue> const &xb) {
        internal_event(q, event_flag);
        external_event(q, 0.0, xb);
    }
    void output_func(double const* q, bool const* event_flag, std::list<PinValue> &yb) {
        assert(event_flag[0] || event_flag[1]);
        PinValue event(ball_output, q[0]);
        yb.push_back(event);
    }

  private:
    bool sample;
    double last_event_time;
    enum { CLIMB = 0, FALL = 1 } phase;
};

class SolutionChecker : public EventListener {
  public:
    SolutionChecker(std::shared_ptr<Hybrid> ball) : ball(ball) {}
    void stateChange(Atomic &model, double t) {
        if (&model == ball.get()) {
            assert(ball1d_soln_ok(t, ball->getState(0)));
        }
    }
    void outputEvent(Atomic &, PinValue &, double) {}
    void inputEvent(Atomic &, PinValue &, double) {}

  private:
    std::shared_ptr<Hybrid> ball;
};

void run_test(ode_system* b, ode_solver* s, event_locator* l) {
    std::cerr << "Testing " << typeid(*s).name() << " , " << typeid(*l).name() << std::endl;
    std::shared_ptr<Hybrid> ball = std::make_shared<Hybrid>(b, s, l);
    std::shared_ptr<sampler> sample = std::make_shared<sampler>(0.01);
    std::shared_ptr<Graph> model = std::make_shared<Graph>();
    model->add_atomic(ball);
    model->add_atomic(sample);
    model->connect(sample->sample_pin, ball);
    model->connect(ball_output, sample);
    std::shared_ptr<SolutionChecker> checker = std::make_shared<SolutionChecker>(ball);
    Simulator sim(model);
    sim.addEventListener(checker);
    while (sim.nextEventTime() < 10.0) {
        sim.execNextEvent();
    }
}

int main() {
    // Test fast algorithm without interpolation
    bouncing_ball* ball = new bouncing_ball();
    run_test(ball, new corrected_euler(ball, 1E-6, 0.01), new fast_event_locator(ball, 1E-7));
    // Test fast algorithm with interpolation
    ball = new bouncing_ball();
    run_test(ball, new corrected_euler(ball, 1E-6, 0.01), new fast_event_locator(ball, 1E-7, true));
    // Test linear algorithm
    ball = new bouncing_ball();
    run_test(ball, new corrected_euler(ball, 1E-6, 0.01), new linear_event_locator(ball, 1E-7));
    // Test RK 45
    ball = new bouncing_ball();
    run_test(ball, new rk_45(ball, 1E-6, 0.01), new linear_event_locator(ball, 1E-7));
    // Test bisection algorithm
    ball = new bouncing_ball();
    run_test(ball, new corrected_euler(ball, 1E-6, 0.01), new bisection_event_locator(ball, 1E-7));
    ball = new bouncing_ball();
    run_test(ball, new rk_45(ball, 1E-6, 0.01), new bisection_event_locator(ball, 1E-7));
    return 0;
}
