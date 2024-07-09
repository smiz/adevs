#include <iostream>
#include "EventTests/modelDescription.h"
#include "adevs/adevs.h"
#include "adevs/fmi.h"
using namespace std;

#define epsilon 1E-6

void test_vars(EventTests* model) {
    cout << model->get_time() << " " << model->get_count() << " "
         << model->get_x() << " " << model->get_y() << " " << endl;
    assert(fabs(model->get_x()) <= 1.0 + epsilon);
    assert(fabs(model->get_y()) <= 1.0 + epsilon);
    assert(model->get_count() >= floor(model->get_time() * 2.0) + 1);
    if (model->get_x() - model->get_x1() > epsilon) {
        assert(model->get_v1());
        assert(model->get_v2());
        assert(!model->get_v3());
        assert(!model->get_v4());
    }
    // 2 x accounts for hysteresis
    else if (model->get_x() - model->get_x1() < -epsilon) {
        assert(!model->get_v1());
        assert(!model->get_v2());
        assert(model->get_v3());
        assert(model->get_v4());
    }
    // 2 x accounts for hysteresis
    if (model->get_x() - model->get_y() > epsilon) {
        assert(model->get_w1());
        assert(model->get_w2());
        assert(!model->get_w3());
        assert(!model->get_w4());
    }
    // 2 x accounts for hysteresis
    else if (model->get_x() - model->get_y() < -epsilon) {
        assert(!model->get_w1());
        assert(!model->get_w2());
        assert(model->get_w3());
        assert(model->get_w4());
    }
}

int main() {
    EventTests* fmi = new EventTests();
    adevs::corrected_euler<int>* solver1 =
        new adevs::corrected_euler<int>(fmi, epsilon / 10.0, 0.01);
    adevs::discontinuous_event_locator<int>* solver2 =
        new adevs::discontinuous_event_locator<int>(fmi, epsilon / 10.0);
    adevs::Hybrid<int>* model = new adevs::Hybrid<int>(fmi, solver1, solver2);
    adevs::Simulator<int>* sim = new adevs::Simulator<int>(model);
    while (sim->nextEventTime() <= 5.0) {
        sim->execNextEvent();
        test_vars(fmi);
    }
    delete sim;
    delete model;
    return 0;
}
