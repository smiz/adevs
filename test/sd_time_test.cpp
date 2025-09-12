#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"


using PinValue = adevs::PinValue<int>;
using pin_t = adevs::pin_t;
using sd_time = adevs::sd_time<double>;
using Atomic = adevs::Atomic<int, adevs::sd_time<double>>;
using EventListener = adevs::EventListener<int, sd_time>;
using Simulator = adevs::Simulator<int, sd_time>;
using Graph = adevs::Graph<int, sd_time>;

class Incr : public Atomic {
  public:
    Incr() : Atomic(), count(0) {}
    sd_time ta() {
        if (count % 2 == 0) {
            return sd_time(count, 1);
        }
        return sd_time(0, count);
    }
    void delta_int() { count++; }
    void delta_ext(sd_time, std::list<PinValue> const &) {}
    void delta_conf(std::list<PinValue> const &) {}
    void output_func(std::list<PinValue> &y) {
        PinValue yy(output_pin, count);
        y.push_back(yy);
    }
    int get_q() const { return count; }

    pin_t const output_pin;

  private:
    int count;
};

class Watch : public Atomic {
  public:
    Watch() : Atomic() {}
    sd_time ta() { return adevs_inf<sd_time>(); }
    void delta_int() { assert(false); }
    void delta_ext(sd_time e, std::list<PinValue> const &xb) {
        int count = (*(xb.begin())).value;
        sd_time expect;
        if (count % 2 == 0) {
            expect = sd_time(count, 1);
        } else {
            expect = sd_time(0, count);
        }
        assert(e == expect);
    }
    void delta_conf(std::list<PinValue> const &) { assert(false); }
    void output_func(std::list<PinValue> &) { assert(false); }
    pin_t const input_pin;
};

class MyEventListener : public EventListener {
  public:
    MyEventListener() : EventListener() {}
    void inputEvent(Atomic &, PinValue &, sd_time) {}
    void outputEvent(Atomic &, PinValue &x, sd_time t) {
        std::cout << "t = " << t << " , y = " << x.value << std::endl;
    }
    void stateChange(Atomic &model, sd_time t) {
        Incr* incr = dynamic_cast<Incr*>(&model);
        if (incr != nullptr) {
            std::cout << "t = " << t << " , q = " << incr->get_q() << " , ta = " << incr->ta()
                      << std::endl;
        } else {
            std::cout << "t = " << t << " , external event"
                      << " , ta = " << model.ta() << std::endl;
        }
    }
};

void test0() {
    std::cout << "TEST 0" << std::endl;
    assert(sd_time(0.0, 0) + sd_time(0.0, 0) == sd_time(0.0, 0));
    assert(sd_time(0, 0) + sd_time(1.0, -1) == sd_time(1.0, -1));
    assert(sd_time(1, 0) + sd_time(1, -1) == sd_time(2, -1));
    assert(sd_time(1, 1) + sd_time(1, -1) == sd_time(2, -1));
    assert(sd_time(1, 1) + sd_time(0, 4) == sd_time(1, 5));
    std::cout << "TEST 0 PASSED" << std::endl;
}

void test1() {
    std::cout << "TEST 1" << std::endl;
    std::shared_ptr<Incr> model = std::make_shared<Incr>();
    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time(10.0, 0)) {
        sim->execNextEvent();
    }
    std::cout << "TEST 1 PASSED" << std::endl;
}

void test2() {
    std::cout << "TEST 2" << std::endl;
    std::shared_ptr<Incr> a = std::make_shared<Incr>();
    std::shared_ptr<Watch> b = std::make_shared<Watch>();

    std::shared_ptr<Graph> model = std::make_shared<Graph>();
    model->add_atomic(a);
    model->add_atomic(b);
    model->connect(a->output_pin, b);

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();

    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time(10.0, 0)) {
        sim->execNextEvent();
    }
    std::cout << "TEST 2 PASSED" << std::endl;
}

int main() {
    test0();
    test1();
    test2();
    return 0;
}
