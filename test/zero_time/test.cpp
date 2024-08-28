#include <cassert>
#include <string>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;

class Parrot : public Atomic<PortValue<string>> {
  public:
    static int const in, out;

    Parrot() : Atomic<PortValue<string>>() {
        k = 0;
        q = "";
    }

    void delta_int() {
        k = (k + 1) % 3;
        q = "";
    }

    void delta_ext(double e, list<PortValue<string>> const &xb) {
        for (auto iter : xb) {
            q += iter.value;
        }
    }

    void delta_conf(list<PortValue<string>> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }

    double ta() {
        if (q == "") {
            return DBL_MAX;
        }
        if (k == 0 || k == 1) {
            return 0.0;
        }
        assert(k == 2);
        return 1.0;
    }

    void output_func(list<PortValue<string>> &yb) {
        PortValue<string> event(out, q);
        yb.push_back(event);
    }

  private:
    int k;
    string q;
};

int const Parrot::in = 0;
int const Parrot::out = 1;

shared_ptr<Digraph<string>> model = nullptr;

class Listener : public EventListener<PortValue<string>> {
  public:
    void outputEvent(Event<PortValue<string>> x, double t) {
        if (x.model == model.get() && x.value.port == Parrot::out) {
            cout << t << " " << x.value.value << endl;
        }
    }
};

int main() {
    model = make_shared<Digraph<string>>();
    shared_ptr<Parrot> p1 = make_shared<Parrot>();
    shared_ptr<Parrot> p2 = make_shared<Parrot>();
    model->add(p1);
    model->add(p2);
    model->couple(model, Parrot::in, p1, Parrot::in);
    model->couple(p1, Parrot::out, model, Parrot::out);
    model->couple(p1, p1->out, p2, p2->in);
    model->couple(p2, p2->out, p1, p1->in);

    Event<PortValue<string>> start;
    start.model = model.get();
    start.value.value = "a";
    start.value.port = Parrot::in;

    Simulator<PortValue<string>> sim(model);
    shared_ptr<Listener> listener = make_shared<Listener>();
    sim.addEventListener(listener);

    list<Event<PortValue<string>>> input;
    input.push_back(start);
    sim.computeNextState(input, 0.0);

    while (sim.nextEventTime() < 10) {
        cout << "***" << endl;
        sim.execNextEvent();
    }
    return 0;
}
