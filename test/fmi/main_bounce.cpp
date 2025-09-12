#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"

using ModelExchange = adevs::ModelExchange<double>;
using Hybrid = adevs::Hybrid<double>;
using Simulator = adevs::Simulator<double>;

static double const epsilon = 1E-7;
static double const err_tol = 1E-3;

class bounce : public ModelExchange {
  public:
    bounce() : ModelExchange("bounce.fmu", err_tol), m_bounce(0), m_resetTime(0.0) {}
    void internal_event(double* q, bool const* state_event) {
        // Apply internal event function of the super class
        ModelExchange::internal_event(q, state_event);
        double a = std::any_cast<double>(get_variable("a"));
        double x = std::any_cast<double>(get_variable("x"));
        int a_above = std::any_cast<int>(get_variable("aAbove"));
        int x_above = std::any_cast<int>(get_variable("xAbove"));
        int go_up = std::any_cast<int>(get_variable("goUp"));
        int go_down = std::any_cast<int>(get_variable("goDown"));
        std::cout << "internal @ " << get_time() << std::endl;
        // Change the direction as needed
        m_bounce++;
        set_variable("a", -a);
        m_resetTime = get_time();
        // Reapply internal event function of the super class
        ModelExchange::internal_event(q, state_event);
        a = std::any_cast<double>(get_variable("a"));
        a = std::any_cast<double>(get_variable("a"));
        x = std::any_cast<double>(get_variable("x"));
        a_above = std::any_cast<int>(get_variable("aAbove"));
        x_above = std::any_cast<int>(get_variable("xAbove"));
        go_up = std::any_cast<int>(get_variable("goUp"));
        go_down = std::any_cast<int>(get_variable("goDown"));
        assert((a > 0.0) == a_above);
        assert((x > 1.5) == x_above);
        assert(go_up == (!a_above && !x_above));
        assert(go_down == (a_above && x_above));
    }
    void print_state() {
        double a = std::any_cast<double>(get_variable("a"));
        double x = std::any_cast<double>(get_variable("x"));
        double der_x = std::any_cast<double>(get_variable("der(x)"));
        int a_above = std::any_cast<int>(get_variable("aAbove"));
        int x_above = std::any_cast<int>(get_variable("xAbove"));
        int go_up = std::any_cast<int>(get_variable("goUp"));
        int go_down = std::any_cast<int>(get_variable("goDown"));
        std::cout << get_time() << " " << x << " " << der_x << " " << a << " " << go_up << " "
                  << go_down << " " << a_above << " " << x_above << " " << std::endl;
    }
    void test_state() {
        double x;
        double xx = std::any_cast<double>(get_variable("x"));
        if (m_bounce % 2 == 0) {
            x = 2.0 * exp(m_resetTime - get_time());
        } else {
            x = exp(get_time() - m_resetTime);
        }
        assert(fabs(x - xx) < err_tol);
    }

  private:
    int m_bounce;
    double m_resetTime;
};

int main() {
    bounce* test_model = new bounce();
    std::shared_ptr<Hybrid> hybrid_model = std::make_shared<Hybrid>(
        test_model, new corrected_euler<double>(test_model, epsilon, 0.001),
        new discontinuous_event_locator<double>(test_model, epsilon));
    // Create the simulator
    Simulator* sim = new Simulator(hybrid_model);
    // Check initial values
    assert(test_model->get_time() == 0.0);
    double x = std::any_cast<double>(test_model->get_variable("x"));
    double der_x = std::any_cast<double>(test_model->get_variable("der(x)"));
    int go_up = std::any_cast<int>(test_model->get_variable("goUp"));
    int go_down = std::any_cast<int>(test_model->get_variable("goDown"));
    std::cout << x << " " << der_x << std::endl;
    assert(fabs(x - 2.0) < err_tol);
    assert(fabs(der_x + 2.0) < err_tol);
    assert(go_up == false);
    assert(go_down == false);
    test_model->print_state();
    // Run the simulation, testing the solution as we go
    while (sim->nextEventTime() <= 2.0) {
        sim->execNextEvent();
        test_model->print_state();
        test_model->test_state();
    }
    delete sim;
    return 0;
}
