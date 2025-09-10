#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"


// using namespace adevs;


class Incr : public adevs::Atomic<int, adevs::sd_time<double>> {
  public:
    Incr() : adevs::Atomic<int, adevs::sd_time<double>>(), count(0) {}
    adevs::sd_time<double> ta() {
        if (count % 2 == 0) {
            return adevs::sd_time<double>(count, 1);
        }
        return adevs::sd_time<double>(0, count);
    }
    void delta_int() { count++; }
    void delta_ext(adevs::sd_time<double>, std::list<adevs::PinValue<int>> const &) {}
    void delta_conf(std::list<adevs::PinValue<int>> const &) {}
    void output_func(std::list<adevs::PinValue<int>> &y) {
    	adevs::PinValue<int> yy(output_pin,count);
        y.push_back(yy);
    }
    int get_q() const { return count; }

    const adevs::pin_t output_pin;
  private:
    int count;
};

class Watch : public adevs::Atomic<int, adevs::sd_time<double>> {
  public:
    Watch() : adevs::Atomic<int, adevs::sd_time<double>>() {}
    adevs::sd_time<double> ta() { return adevs_inf<adevs::sd_time<double>>(); }
    void delta_int() { assert(false); }
    void delta_ext(adevs::sd_time<double> e, std::list<adevs::PinValue<int>> const &xb) {
        int count = (*(xb.begin())).value;
        adevs::sd_time<double> expect;
        if (count % 2 == 0) {
            expect = adevs::sd_time<double>(count, 1);
        } else {
            expect = adevs::sd_time<double>(0, count);
        }
        assert(e == expect);
    }
    void delta_conf(std::list<adevs::PinValue<int>> const &) { assert(false); }
    void output_func(std::list<adevs::PinValue<int>> &) { assert(false); }
    const adevs::pin_t input_pin;
};

class MyEventListener : public adevs::EventListener<int, adevs::sd_time<double>> {
  public:
    MyEventListener() : adevs::EventListener<int, adevs::sd_time<double>>() {}
    void inputEvent(adevs::Atomic<int,adevs::sd_time<double>>&, adevs::PinValue<int>&, adevs::sd_time<double>) {}
    void outputEvent(adevs::Atomic<int,adevs::sd_time<double>>&, adevs::PinValue<int>& x, adevs::sd_time<double> t) {
        std::cout << "t = " << t << " , y = " << x.value << std::endl;
    }
    void stateChange(adevs::Atomic<int, adevs::sd_time<double>>& model, adevs::sd_time<double> t) {
        Incr* incr = dynamic_cast<Incr*>(&model);
        if (incr != nullptr) {
            std::cout << "t = " << t << " , q = " << incr->get_q()
                 << " , ta = " << incr->ta() << std::endl;
        } else {
            std::cout << "t = " << t << " , external event"
                 << " , ta = " << model.ta() << std::endl;
        }
    }
};

void test0() {
    std::cout << "TEST 0" << std::endl;
    assert(adevs::sd_time<>(0.0, 0) + adevs::sd_time<>(0.0, 0) == adevs::sd_time<>(0.0, 0));
    assert(adevs::sd_time<>(0, 0) + adevs::sd_time<>(1.0, -1) == adevs::sd_time<>(1.0, -1));
    assert(adevs::sd_time<>(1, 0) + adevs::sd_time<>(1, -1) == adevs::sd_time<>(2, -1));
    assert(adevs::sd_time<>(1, 1) + adevs::sd_time<>(1, -1) == adevs::sd_time<>(2, -1));
    assert(adevs::sd_time<>(1, 1) + adevs::sd_time<>(0, 4) == adevs::sd_time<>(1, 5));
    std::cout << "TEST 0 PASSED" << std::endl;
}

void test1() {
    std::cout << "TEST 1" << std::endl;
    std::shared_ptr<Incr> model = std::make_shared<Incr>();
    std::shared_ptr<adevs::Simulator<int, adevs::sd_time<>>> sim =
        std::make_shared<adevs::Simulator<int, adevs::sd_time<>>>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < adevs::sd_time<>(10.0, 0)) {
        sim->execNextEvent();
    }
    std::cout << "TEST 1 PASSED" << std::endl;
}

void test2() {
    std::cout << "TEST 2" << std::endl;
    std::shared_ptr<Incr> a = std::make_shared<Incr>();
    std::shared_ptr<Watch> b = std::make_shared<Watch>();

    std::shared_ptr<adevs::Graph<int, adevs::sd_time<>>> model =
        std::make_shared<adevs::Graph<int, adevs::sd_time<>>>();
    model->add_atomic(a);
    model->add_atomic(b);
    model->connect(a->output_pin, b);

    std::shared_ptr<adevs::Simulator<int, adevs::sd_time<>>> sim =
        std::make_shared<adevs::Simulator<int, adevs::sd_time<>>>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();

    sim->addEventListener(listener);
    while (sim->nextEventTime() < adevs::sd_time<>(10.0, 0)) {
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
