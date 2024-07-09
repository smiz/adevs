#include <vector>
#include "Relay.h"
using namespace std;
using namespace adevs;

class Start : public adevs::Atomic<IO_Type> {
  public:
    Start() : adevs::Atomic<IO_Type>() { go = true; }
    void delta_int() { go = false; }
    void delta_ext(double, adevs::Bag<IO_Type> const &) {}
    void delta_conf(adevs::Bag<IO_Type> const &) {}
    double ta() { return (go) ? 0.0 : adevs_inf<double>(); }
    void output_func(adevs::Bag<IO_Type> &y) { y.push_back(IO_Type(0, 1)); }
    void gc_output(adevs::Bag<IO_Type> &) {}

  private:
    bool go;
};

int inputs = 0, outputs = 0;
Relay* r1 = new Relay();
Relay* r2 = new Relay();
Start* s = new Start();

class Listener : public EventListener<IO_Type> {
  public:
    Listener() : EventListener<IO_Type>() {}
    void inputEvent(Event<IO_Type> x, double t) {
        inputs++;
        assert(inputs == t + 1);
        if ((int)(t) % 2 == 0) {
            assert(x.model == r1);
        } else {
            assert(x.model == r2);
        }
    }
    void outputEvent(Event<IO_Type> x, double t) {
        outputs++;
        assert(outputs == t + 1);
        if (t == 0.0) {
            assert(x.model == s);
        } else if ((int)(t) % 2 == 0) {
            assert(x.model == r2);
        } else {
            assert(x.model == r1);
        }
    }
};

int main() {
    Digraph<int>* d = new Digraph<int>();
    d->add(r1);
    d->add(r2);
    d->add(s);
    d->couple(s, 0, r1, 0);
    d->couple(r1, 1, r2, 0);
    d->couple(r2, 1, r1, 0);
    // Create the simulator and add the listener
    Simulator<IO_Type>* sim = new Simulator<IO_Type>(d);
    sim->addEventListener(new Listener());
    for (int i = 0; i < 10; i++) {
        sim->execNextEvent();
    }
    assert(inputs == 10);
    assert(outputs == 10);
    // Done
    return 0;
}
