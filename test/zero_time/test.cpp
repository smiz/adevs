#include <cassert>
#include <string>
#include "adevs.h"
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
    void delta_ext(double e, Bag<PortValue<string>> const &xb) {
        Bag<PortValue<string>>::const_iterator iter;
        for (iter = xb.begin(); iter != xb.end(); iter++) {
            q += (*iter).value;
        }
    }
    void delta_conf(Bag<PortValue<string>> const &xb) {
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
    void output_func(Bag<PortValue<string>> &yb) {
        PortValue<string> event(out, q);
        yb.insert(event);
    }
    void gc_output(Bag<PortValue<string>> &) {}
    ~Parrot() {}

  private:
    int k;
    string q;
};

int const Parrot::in = 0;
int const Parrot::out = 1;

Digraph<string>* model;

class Listener : public EventListener<PortValue<string>> {
  public:
    void outputEvent(Event<PortValue<string>> x, double t) {
        if (x.model == model && x.value.port == Parrot::out) {
            cout << t << " " << x.value.value << endl;
        }
    }
};

int main() {
    model = new Digraph<string>();
    Parrot* p1 = new Parrot();
    Parrot* p2 = new Parrot();
    model->add(p1);
    model->add(p2);
    model->couple(model, Parrot::in, p1, Parrot::in);
    model->couple(p1, Parrot::out, model, Parrot::out);
    model->couple(p1, p1->out, p2, p2->in);
    model->couple(p2, p2->out, p1, p1->in);
    Event<PortValue<string>> start;
    start.model = model;
    start.value.value = "a";
    start.value.port = Parrot::in;
    Simulator<PortValue<string>> sim(model);
    sim.addEventListener(new Listener());
    Bag<Event<PortValue<string>>> input;
    input.insert(start);
    sim.computeNextState(input, 0.0);
    while (sim.nextEventTime() < 10) {
        cout << "***" << endl;
        sim.execNextEvent();
    }
    return 0;
}
