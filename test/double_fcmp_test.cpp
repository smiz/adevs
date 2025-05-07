#include <cassert>
#include <iostream>
#include "adevs/adevs.h"
using namespace std;
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
            sigma = DBL_MAX;
        }
    }
    void delta_ext(double_fcmp, list<PinValue<char>> const &) { sigma = DBL_MAX; }
    void delta_conf(list<PinValue<char>> const &) { sigma = DBL_MAX; }
    void output_func(list<PinValue<char>> &y) {
        PinValue<char> output(0,'a');
        y.push_back(output);
    }
    int getTickCount() { return count; }

  private:
    int ticks;
    int count;
    double_fcmp sigma;
};

void test1() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

void test2() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        sim.execNextEvent();
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
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    shared_ptr<MyEventListener> listener = make_shared<MyEventListener>();
    sim.addEventListener(listener);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        assert(listener->t_last == sim.nextEventTime());
        sim.execNextEvent();
    }
    assert(listener->count == 10);
    assert(g->getTickCount() == 10);
}

void test4() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    sim.setNextTime(5.0);
    sim.computeNextState();
    assert(sim.nextEventTime() == 10.0);
    sim.setNextTime(6.0);
    sim.computeNextState();
    assert(sim.nextEventTime() == 10.0);
    sim.setNextTime(sim.nextEventTime());
    sim.computeNextOutput();
    sim.computeNextState();
    assert(sim.nextEventTime() == 20.0);
    assert(g->getTickCount() == 1);
    sim.computeNextOutput();
    assert(sim.nextEventTime() == 20.0);
    sim.setNextTime(12.0);
    sim.computeNextState();
    assert(sim.nextEventTime() == 20.0);
    assert(g->getTickCount() == 1);
    sim.execNextEvent();
    assert(g->getTickCount() == 2);
    assert(sim.nextEventTime() == 30.0);
}

void test5() {
    PinValue<char> input(0,'a');
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    auto graph = make_shared<Graph<char,double_fcmp>>();
    graph->add_atomic(g);
    graph->connect(0,g);
    Simulator<char, double_fcmp> sim(graph);
    sim.injectInput(input);
    sim.setNextTime(5.0);
    sim.computeNextState();
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
