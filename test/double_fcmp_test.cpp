#include <cassert>
#include <iostream>
#include "adevs/adevs.h"

// using namespace adevs;

double adevs::double_fcmp::epsilon = DBL_EPSILON;

class genr : public adevs::Atomic<char, adevs::double_fcmp> {
  public:
    genr(double period, int ticks)
        : adevs::Atomic<char, adevs::double_fcmp>(), ticks(ticks), count(0), sigma(period) {}
    adevs::double_fcmp ta() { return sigma; }
    void delta_int() {
        count++;
        assert(count <= ticks);
        if (count == ticks) {
            sigma = adevs_inf<adevs::double_fcmp>();
        }
    }
    void delta_ext(adevs::double_fcmp, std::list<adevs::PinValue<char>> const &) { sigma = DBL_MAX; }
    void delta_conf(std::list<adevs::PinValue<char>> const &) { sigma = DBL_MAX; }
    void output_func(std::list<adevs::PinValue<char>> &y) {
    	adevs::PinValue<char> output(output_pin,'a');
        y.push_back(output);
    }
    int getTickCount() { return count; }

    const adevs::pin_t output_pin;
    const adevs::pin_t input_pin;
  private:
    int ticks;
    int count;
    adevs::double_fcmp sigma;
};

void test1() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    adevs::Simulator<char, adevs::double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

void test2() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    adevs::Simulator<char, adevs::double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        sim.computeNextState();
    }
    assert(g->getTickCount() == 10);
}

class MyEventListener : public adevs::EventListener<char, adevs::double_fcmp> {
  public:
    MyEventListener() {
        count = 0;
        t_last = 0.0;
    }
    void inputEvent(adevs::Atomic<char,adevs::double_fcmp>&, adevs::PinValue<char>&, adevs::double_fcmp) {}
    void outputEvent(adevs::Atomic<char,adevs::double_fcmp>&, adevs::PinValue<char>&, adevs::double_fcmp t) {
        count++;
        t_last = t;
    }
    void stateChange(adevs::Atomic<char,adevs::double_fcmp>&, adevs::double_fcmp) {}

    int count;
    adevs::double_fcmp t_last;
};

void test3() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    adevs::Simulator<char, adevs::double_fcmp> sim(g);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim.addEventListener(listener);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        assert(listener->t_last == sim.nextEventTime());
        sim.computeNextState();
    }
    assert(listener->count == 10);
    assert(g->getTickCount() == 10);
}

void test4() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    adevs::Simulator<char, adevs::double_fcmp> sim(g);
    sim.setNextTime(5.0);
    sim.execNextEvent();
    assert(sim.nextEventTime() == 10.0);
    sim.setNextTime(6.0);
    sim.execNextEvent();
    assert(sim.nextEventTime() == 10.0);
    sim.setNextTime(sim.nextEventTime());
    sim.computeNextOutput();
    sim.computeNextState();
    assert(sim.nextEventTime() == 20.0);
    assert(g->getTickCount() == 1);
    sim.computeNextOutput();
    assert(sim.nextEventTime() == 20.0);
    sim.setNextTime(12.0);
    sim.computeNextOutput();
    sim.computeNextState();
    assert(sim.nextEventTime() == 20.0);
    assert(g->getTickCount() == 1);
    sim.execNextEvent();
    assert(g->getTickCount() == 2);
    assert(sim.nextEventTime() == 30.0);
}

void test5() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    auto graph = std::make_shared<adevs::Graph<char,adevs::double_fcmp>>();
    adevs::PinValue<char> input(g->input_pin,'a');
    graph->add_atomic(g);
    graph->connect(g->input_pin,g);
    adevs::Simulator<char, adevs::double_fcmp> sim(graph);
    sim.injectInput(input);
    sim.setNextTime(5.0);
    sim.execNextEvent();
    assert(sim.nextEventTime() == DBL_MAX);
    assert(g->getTickCount() == 0);
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}
