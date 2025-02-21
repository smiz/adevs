#include <cassert>
#include <iostream>
#include "adevs/models.h"
#include "adevs/simulator.h"
using namespace std;
using namespace adevs;

class genr : public Atomic<char> {
  public:
    genr(double period, int ticks)
        : Atomic<char>(), ticks(ticks), count(0), sigma(period) {}
    double ta() { return sigma; }
    void delta_int() {
        count++;
        assert(count <= ticks);
        if (count == ticks) {
            sigma = adevs_inf<double>();
        }
    }
    void delta_ext(double, list<PinValue<char>> const &) { sigma = adevs_inf<double>(); }
    void delta_conf(list<PinValue<char>> const &) { sigma = adevs_inf<double>(); }
    void output_func(list<PinValue<char>> &y) {
        PinValue<char> output(0,'a');
        y.push_back(output);
    }
    int getTickCount() { return count; }

  private:
    int ticks;
    int count;
    double sigma;
};

void test1() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

void test2() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.computeNextOutput();
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

class MyEventListener : public EventListener<char> {
  public:
    MyEventListener() {
        count = 0;
        t_last = 0.0;
    }
    void outputEvent(Atomic<char>&, PinValue<char>& x, double t) {
        count++;
        t_last = t;
    }
    void inputEvent(Atomic<char>&, PinValue<char>& x, double t) {}
    void stateChange(Atomic<char>&, double t) {}
    int count;
    double t_last;
};

void test3() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
    shared_ptr<MyEventListener> listener = make_shared<MyEventListener>();
    sim.addEventListener(listener);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.computeNextOutput();
        assert(listener->t_last == sim.nextEventTime());
        sim.execNextEvent();
    }
    assert(listener->count == 10);
    assert(g->getTickCount() == 10);
}

void test4() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
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
    shared_ptr<Graph<char>> graph = make_shared<Graph<char>>();
    graph->add_atomic(g);
    graph->connect(0,g);
    Simulator<char> sim(graph);
    sim.setNextTime(5.0);
    sim.injectInput(input);
    sim.computeNextState();
    assert(sim.nextEventTime() == adevs_inf<double>());
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
