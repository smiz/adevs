#include "relay.h"


using Simulator = adevs::Simulator<int>;
using EventListener = adevs::EventListener<int>;
using Graph = adevs::Graph<int>;
using PinValue = adevs::PinValue<int>;

class Start : public Atomic {
  public:
    Start() : Atomic() { go = true; }

    void delta_int() { go = false; }

    void delta_ext(double, std::list<PinValue> const &) {}

    void delta_conf(std::list<PinValue> const &) {}

    double ta() { return (go) ? 0.0 : adevs_inf<double>(); }

    void output_func(std::list<PinValue> &y) { y.push_back(PinValue(out, 1)); }

    pin_t const out;

  private:
    bool go;
};

int inputs = 0, outputs = 0;
std::shared_ptr<Relay> r1 = std::make_shared<Relay>();
std::shared_ptr<Relay> r2 = std::make_shared<Relay>();
std::shared_ptr<Start> s = std::make_shared<Start>();

class Listener : public EventListener {
  public:
    Listener() : EventListener() {}
    void inputEvent(Atomic &model, PinValue &, double t) {
        inputs++;
        assert(inputs == t + 1);
        if ((int)(t) % 2 == 0) {
            assert(&model == r1.get());
        } else {
            assert(&model == r2.get());
        }
    }
    void outputEvent(Atomic &model, PinValue &, double t) {
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
    void stateChange(Atomic &, double) {}
};

int main() {
    std::shared_ptr<Graph> d = std::make_shared<Graph>();
    d->add_atomic(r1);
    d->add_atomic(r2);
    d->add_atomic(s);
    d->connect(s->out, r1->in);
    d->connect(r1->in, r1);
    d->connect(r1->out, r2->in);
    d->connect(r2->out, r1->in);
    d->connect(r2->in, r2);

    std::shared_ptr<Listener> listener = std::make_shared<Listener>();
    // Create the simulator and add the listener
    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(d);
    sim->addEventListener(listener);

    for (int i = 0; i < 10; i++) {
        sim->execNextEvent();
    }
    assert(inputs == 10);
    assert(outputs == 10);
    // Done
    return 0;
}
