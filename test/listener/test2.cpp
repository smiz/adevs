#include "relay.h"
using namespace std;
using namespace adevs;

class Start : public adevs::Atomic<int> {
  public:
    Start() : adevs::Atomic<int>() { go = true; }

    void delta_int() { go = false; }

    void delta_ext(double, list<PinValue<int>> const &) {}

    void delta_conf(list<PinValue<int>> const &) {}

    double ta() { return (go) ? 0.0 : adevs_inf<double>(); }

    void output_func(list<PinValue<int>> &y) { y.push_back(PinValue<int>(out, 1)); }

    const pin_t out;
  private:
    bool go;
};

int inputs = 0, outputs = 0;
shared_ptr<Relay> r1 = make_shared<Relay>();
shared_ptr<Relay> r2 = make_shared<Relay>();
shared_ptr<Start> s = make_shared<Start>();

class Listener : public EventListener<int> {
  public:
    Listener() : EventListener<int>() {}
    void inputEvent(Atomic<int>& model, PinValue<int>&, double t) {
        inputs++;
        assert(inputs == t + 1);
        if ((int)(t) % 2 == 0) {
            assert(&model == r1.get());
        } else {
            assert(&model == r2.get());
        }
    }
    void outputEvent(Atomic<int>& model, PinValue<int>&, double t) {
        outputs++;
        assert(outputs == t + 1);
        if (t == 0.0) {
            assert(&model == s.get());
        } else if ((int)(t) % 2 == 0) {
            assert(&model == r2.get());
        } else {
            assert(&model == r1.get());
        }
    }
    void stateChange(Atomic<int>&, double) {}
};

int main() {
    shared_ptr<Graph<int>> d = make_shared<Graph<int>>();
    d->add_atomic(r1);
    d->add_atomic(r2);
    d->add_atomic(s);
    d->connect(s->out,r1->in);
    d->connect(r1->in,r1);
    d->connect(r1->out,r2->in);
    d->connect(r2->out,r1->in);
    d->connect(r2->in,r2);

    shared_ptr<Listener> listener = make_shared<Listener>();
    // Create the simulator and add the listener
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(d);
    sim->addEventListener(listener);

    for (int i = 0; i < 10; i++) {
        sim->execNextEvent();
    }
    assert(inputs == 10);
    assert(outputs == 10);
    // Done
    return 0;
}
