#include <cassert>
#include <iostream>
#include "adevs/adevs.h"
#include "adevs/fmi.h"
#include "eventIter/modelDescription.h"
using namespace std;
using namespace adevs;

static double const epsilon = 1E-3;

class TestModel : public eventIter {
  public:
    TestModel() : eventIter() {}
};

void print(TestModel* model) {
    double cfloor = model->get_c() - floor(model->get_c());
    cout << model->get_time() << " " << model->get_c() << " "
         << model->get_floorc() << " " << model->get_high() << endl;
    assert(cfloor <= 0.5 + epsilon);
    assert(cfloor <= 0.25 + epsilon || model->get_high() == 1);
    assert(cfloor >= 0.25 - epsilon || model->get_high() == 0);
    assert(model->get_floorc() == cfloor);
}

void test(bool interpolate) {
    TestModel* test_model = new TestModel();
    Hybrid<double>* hybrid_model = new Hybrid<double>(
        test_model, new corrected_euler<double>(test_model, 1E-5, 0.001),
        new fast_event_locator<double>(test_model, 1E-6, interpolate));
    // Create the simulator
    Simulator<double>* sim = new Simulator<double>(hybrid_model);
    // Check initial values
    print(test_model);
    // Run the simulation, testing the solution as we go
    while (sim->nextEventTime() <= 5.0) {
        sim->execNextEvent();
        print(test_model);
    }
    delete sim;
    delete hybrid_model;
}

int main() {
    test(false);
    test(true);
    return 0;
}
