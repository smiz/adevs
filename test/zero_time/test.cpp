#include <cassert>
#include <string>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;

class Parrot : public Atomic<string> {
  public:
    pin_t in, out;

    Parrot() : Atomic<string>() {
        k = 0;
        q = "";
    }

    void delta_int() {
        k = (k + 1) % 3;
        q = "";
    }

    void delta_ext(double, list<PinValue<string>> const &xb) {
        for (auto iter : xb) {
            q += iter.value;
        }
    }

    void delta_conf(list<PinValue<string>> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }

    double ta() {
        if (q == "") {
            return adevs_inf<double>();
        }
        if (k == 0 || k == 1) {
            return 0.0;
        }
        assert(k == 2);
        return 1.0;
    }

    void output_func(list<PinValue<string>> &yb) {
        PinValue<string> event(out, q);
        yb.push_back(event);
    }

  private:
    int k;
    string q;
};

shared_ptr<Parrot> p1;

class Listener : public EventListener<string> {
  public:
    void stateChange(Atomic<string>&, double) {}
    void inputEvent(Atomic<string>&, PinValue<string>&, double) {}
    void outputEvent(Atomic<string>&, PinValue<string>& x, double t) {
        if (x.pin == p1->out) {
            cout << t << " " << x.value << endl;
        }
    }
};

int main() {
    auto model = make_shared<Graph<string>>();
    p1 = make_shared<Parrot>();
    shared_ptr<Parrot> p2 = make_shared<Parrot>();
    model->add_atomic(p1);
    model->add_atomic(p2);
    p1->in = model->add_pin();
    p1->out = model->add_pin();
    p2->in = model->add_pin();
    p2->out = model->add_pin();
    model->connect(p1->in,p1);
    model->connect(p2->in,p2);
    model->connect(p1->out,p2->in);
    model->connect(p2->out,p1->in);
    adevs::PinValue<string> start;
    start.pin = p1->in;
    start.value = "a";
    Simulator<string> sim(model);
    shared_ptr<Listener> listener = make_shared<Listener>();
    sim.addEventListener(listener);
    sim.setNextTime(0.0);
    sim.injectInput(start);
    sim.computeNextState();
    while (sim.nextEventTime() < 10) {
        cout << "***" << endl;
        sim.execNextEvent();
    }
    return 0;
}
