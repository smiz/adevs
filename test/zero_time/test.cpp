#include <cassert>
#include <string>
#include "adevs/adevs.h"


using pin_t = adevs::pin_t;
using Graph = adevs::Graph<std::string>;
using Atomic = adevs::Atomic<std::string>;
using PinValue = adevs::PinValue<std::string>;
using EventListener = adevs::EventListener<std::string>;
using Simulator = adevs::Simulator<std::string>;

class Parrot : public Atomic {
  public:
    pin_t in, out;

    Parrot() : Atomic() {
        k = 0;
        q = "";
    }

    void delta_int() {
        k = (k + 1) % 3;
        q = "";
    }

    void delta_ext(double, std::list<PinValue> const &xb) {
        for (auto iter : xb) {
            q += iter.value;
        }
    }

    void delta_conf(std::list<PinValue> const &xb) {
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

    void output_func(std::list<PinValue> &yb) {
        PinValue event(out, q);
        yb.push_back(event);
    }

  private:
    int k;
    std::string q;
};

std::shared_ptr<Parrot> p1;

class Listener : public EventListener {
  public:
    void stateChange(Atomic &, double) {}
    void inputEvent(Atomic &, PinValue &, double) {}
    void outputEvent(Atomic &, PinValue &x, double t) {
        if (x.pin == p1->out) {
            std::cout << t << " " << x.value << std::endl;
        }
    }
};

int main() {
    auto model = std::make_shared<Graph>();
    p1 = std::make_shared<Parrot>();
    std::shared_ptr<Parrot> p2 = std::make_shared<Parrot>();
    model->add_atomic(p1);
    model->add_atomic(p2);
    model->connect(p1->in, p1);
    model->connect(p2->in, p2);
    model->connect(p1->out, p2->in);
    model->connect(p2->out, p1->in);
    PinValue start;
    start.pin = p1->in;
    start.value = "a";
    Simulator sim(model);
    std::shared_ptr<Listener> listener = std::make_shared<Listener>();
    sim.addEventListener(listener);
    sim.setNextTime(0.0);
    sim.injectInput(start);
    sim.execNextEvent();
    while (sim.nextEventTime() < 10) {
        std::cout << "***" << std::endl;
        sim.execNextEvent();
    }
    return 0;
}
