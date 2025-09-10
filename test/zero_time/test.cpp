#include <cassert>
#include <string>
#include "adevs/adevs.h"


// using namespace adevs;

class Parrot : public adevs::Atomic<std::string> {
  public:
		adevs::pin_t in, out;

    Parrot() : adevs::Atomic<std::string>() {
        k = 0;
        q = "";
    }

    void delta_int() {
        k = (k + 1) % 3;
        q = "";
    }

    void delta_ext(double, std::list<adevs::PinValue<std::string>> const &xb) {
        for (auto iter : xb) {
            q += iter.value;
        }
    }

    void delta_conf(std::list<adevs::PinValue<std::string>> const &xb) {
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

    void output_func(std::list<adevs::PinValue<std::string>> &yb) {
    	adevs::PinValue<std::string> event(out, q);
        yb.push_back(event);
    }

  private:
    int k;
    std::string q;
};

std::shared_ptr<Parrot> p1;

class Listener : public adevs::EventListener<std::string> {
  public:
    void stateChange(adevs::Atomic<std::string>&, double) {}
    void inputEvent(adevs::Atomic<std::string>&, adevs::PinValue<std::string>&, double) {}
    void outputEvent(adevs::Atomic<std::string>&, adevs::PinValue<std::string>& x, double t) {
        if (x.pin == p1->out) {
            std::cout << t << " " << x.value << std::endl;
        }
    }
};

int main() {
    auto model = std::make_shared<adevs::Graph<std::string>>();
    p1 = std::make_shared<Parrot>();
    std::shared_ptr<Parrot> p2 = std::make_shared<Parrot>();
    model->add_atomic(p1);
    model->add_atomic(p2);
    model->connect(p1->in,p1);
    model->connect(p2->in,p2);
    model->connect(p1->out,p2->in);
    model->connect(p2->out,p1->in);
    adevs::PinValue<std::string> start;
    start.pin = p1->in;
    start.value = "a";
    adevs::Simulator<std::string> sim(model);
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
