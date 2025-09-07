#include <cassert>
#include <iostream>
#include "adevs/adevs.h"

using namespace adevs;

double double_fcmp::epsilon = DBL_EPSILON;

class genr : public Atomic<char, double_fcmp> {
  public:
    genr(double period, int ticks)
        : Atomic<char, double_fcmp>(), ticks(ticks), count(0), sigma(period) {}
    double_fcmp ta() { return sigma; }
    void delta_int() {
        count++;
        assert(count <= ticks);
        if (count == ticks) {
            sigma = adevs_inf<double_fcmp>();
        }
    }
    void delta_ext(double_fcmp, std::list<PinValue<char>> const &) { sigma = DBL_MAX; }
    void delta_conf(std::list<PinValue<char>> const &) { sigma = DBL_MAX; }
    void output_func(std::list<PinValue<char>> &y) {
        PinValue<char> output(output_pin,'a');
        y.push_back(output);
    }
    int getTickCount() { return count; }

    const pin_t output_pin;
    const pin_t input_pin;
  private:
    int ticks;
    int count;
    double_fcmp sigma;
};

void test1() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

void test2() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        sim.computeNextState();
    }
    assert(g->getTickCount() == 10);
}

class MyEventListener : public EventListener<char, double_fcmp> {
  public:
    MyEventListener() {
        count = 0;
        t_last = 0.0;
    }
    void inputEvent(Atomic<char,double_fcmp>&, PinValue<char>&, double_fcmp) {}
    void outputEvent(Atomic<char,double_fcmp>&, PinValue<char>&, double_fcmp t) {
        count++;
        t_last = t;
    }
    void stateChange(Atomic<char,double_fcmp>&, double_fcmp) {}

    int count;
    double_fcmp t_last;
};

void test3() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
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
    Simulator<char, double_fcmp> sim(g);
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
    auto graph = std::make_shared<Graph<char,double_fcmp>>();
    PinValue<char> input(g->input_pin,'a');
    graph->add_atomic(g);
    graph->connect(g->input_pin,g);
    Simulator<char, double_fcmp> sim(graph);
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
