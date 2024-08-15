#include "relay.h"


class Start : public adevs::Atomic<IO_Type> {
  public:
    Start() : adevs::Atomic<IO_Type>() { go = true; }

    void delta_int() { go = false; }

    void delta_ext(double, adevs::Bag<IO_Type> const &) {}

    void delta_conf(adevs::Bag<IO_Type> const &) {}

    double ta() { return (go) ? 0.0 : adevs_inf<double>(); }

    void output_func(adevs::Bag<IO_Type> &y) { y.push_back(IO_Type(0, 1)); }


  private:
    bool go;
};

int inputs = 0, outputs = 0;
shared_ptr<Relay> r1 = make_shared<Relay>();
shared_ptr<Relay> r2 = make_shared<Relay>();
shared_ptr<Start> s = make_shared<Start>();

class Listener : public EventListener<IO_Type> {
  public:
    Listener() : EventListener<IO_Type>() {}
    void inputEvent(Event<IO_Type> x, double t) {
        inputs++;
        assert(inputs == t + 1);
        if ((int)(t) % 2 == 0) {
            assert(x.model == r1.get());
        } else {
            assert(x.model == r2.get());
        }
    }
    void outputEvent(Event<IO_Type> x, double t) {
        outputs++;
        assert(outputs == t + 1);
        if (t == 0.0) {
            assert(x.model == s.get());
        } else if ((int)(t) % 2 == 0) {
            assert(x.model == r2.get());
        } else {
            assert(x.model == r1.get());
        }
    }
};

int main() {
    shared_ptr<Digraph<int>> d = make_shared<Digraph<int>>();
    d->add(r1);
    d->add(r2);
    d->add(s);
    d->couple(s, 0, r1, 0);
    d->couple(r1, 1, r2, 0);
    d->couple(r2, 1, r1, 0);

    shared_ptr<Listener> listener = make_shared<Listener>();
    // Create the simulator and add the listener
    shared_ptr<Simulator<IO_Type>> sim = make_shared<Simulator<IO_Type>>(d);
    sim->addEventListener(listener);

    for (int i = 0; i < 10; i++) {
        sim->execNextEvent();
    }
    assert(inputs == 10);
    assert(outputs == 10);
    // Done
    return 0;
}
