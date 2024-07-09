#include "CartModel.h"
#include "PIDControl.h"
#include "TrajRecorder.h"
#include "adevs.h"
using namespace std;
using namespace adevs;

int main() {
    PIDControl* pid = new PIDControl();
    CartModel* cart = new CartModel();
    Hybrid<double>* hysim =
        new Hybrid<double>(cart, new corrected_euler<double>(cart, 1E-8, 0.001),
                           new linear_event_locator<double>(cart, 1E-10));
    SimpleDigraph<double>* model = new SimpleDigraph<double>();
    model->add(pid);
    model->add(hysim);
    model->couple(pid, hysim);
    model->couple(hysim, pid);
    Simulator<double>* sim = new Simulator<double>(model);
    TrajRecorder* l = new TrajRecorder(cart, hysim);
    sim->addEventListener(l);
    while (sim->nextEventTime() < 5.0) {
        sim->execNextEvent();
    }
    delete sim;
    delete model;
    delete l;
    return 0;
}
