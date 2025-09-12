#include <iostream>
#include "adevs/adevs.h"
#include "adevs/solvers/fmi.h"

using ModelExchange = adevs::ModelExchange<>;
using Simulator = adevs::Simulator<>;
using corrected_euler = adevs::corrected_euler<>;
using discontinuous_event_locator = adevs::discontinuous_event_locator<>;
using Hybrid = adevs::Hybrid;

#define epsilon 1E-6

void test_vars(ModelExchange* model) {
    double x = std::any_cast<double>(model->get_variable("x"));
    double x1 = std::any_cast<double>(model->get_variable("x1"));
    double y = std::any_cast<double>(model->get_variable("y"));
    int count = std::any_cast<int>(model->get_variable("count"));
    bool v1 = std::any_cast<int>(model->get_variable("v1"));
    bool v2 = std::any_cast<int>(model->get_variable("v2"));
    bool v3 = std::any_cast<int>(model->get_variable("v3"));
    bool v4 = std::any_cast<int>(model->get_variable("v4"));
    bool w1 = std::any_cast<int>(model->get_variable("w1"));
    bool w2 = std::any_cast<int>(model->get_variable("w2"));
    bool w3 = std::any_cast<int>(model->get_variable("w3"));
    bool w4 = std::any_cast<int>(model->get_variable("w4"));
    std::cout << model->get_time() << " " << count << " " << x << " " << y << std::endl;
    assert(fabs(x) <= 1.0 + epsilon);
    assert(fabs(y) <= 1.0 + epsilon);
    assert(count >= floor(model->get_time() * 2.0) + 1);
    if (x - x1 > epsilon) {
        assert(v1);
        assert(v2);
        assert(!v3);
        assert(!v4);
    }
    // 2 x accounts for hysteresis
    else if (x - x1 < -epsilon) {
        assert(!v1);
        assert(!v2);
        assert(v3);
        assert(v4);
    }
    // 2 x accounts for hysteresis
    if (x - y > epsilon) {
        assert(w1);
        assert(w2);
        assert(!w3);
        assert(!w4);
    }
    // 2 x accounts for hysteresis
    else if (x - y < -epsilon) {
        assert(!w1);
        assert(!w2);
        assert(w3);
        assert(w4);
    }
}

int main() {
    double const err_tol = 1E-8;
    auto fmi = new ModelExchange("EventTests.fmu", err_tol);
    corrected_euler* solver1 = new corrected_euler(fmi, err_tol, 0.01);
    discontinuous_event_locator* solver2 = new discontinuous_event_locator(fmi, err_tol);
    std::shared_ptr<Hybrid> model = std::make_shared<Hybrid>(fmi, solver1, solver2);
    Simulator* sim = new Simulator(model);
    while (sim->nextEventTime() <= 5.0) {
        sim->execNextEvent();
        test_vars(fmi);
    }
    delete sim;
    return 0;
}
