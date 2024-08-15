#include <cassert>
#include <iostream>
#include "adevs/adevs.h"
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
            sigma = DBL_MAX;
        }
    }
    void delta_ext(double, Bag<char> const &) { sigma = DBL_MAX; }
    void delta_conf(Bag<char> const &) { sigma = DBL_MAX; }
    void output_func(Bag<char> &y) { y.push_back('a'); }
    int getTickCount() { return count; }

  private:
    int ticks;
    int count;
    double sigma;
};

void test1() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

void test2() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
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
    void outputEvent(Event<char> x, double t) {
        count++;
        t_last = t;
    }
    int count;
    double t_last;
};

void test3() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
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
    Simulator<char> sim(g);
    Bag<Event<char>> input;
    sim.computeNextState(input, 5.0);
    assert(sim.nextEventTime() == 10.0);
    sim.computeNextState(input, 6.0);
    assert(sim.nextEventTime() == 10.0);
    sim.computeNextOutput();
    sim.computeNextState(input, sim.nextEventTime());
    assert(sim.nextEventTime() == 20.0);
    assert(g->getTickCount() == 1);
    sim.computeNextOutput();
    assert(sim.nextEventTime() == 20.0);
    sim.computeNextState(input, 12.0);
    assert(sim.nextEventTime() == 20.0);
    assert(g->getTickCount() == 1);
    sim.execNextEvent();
    assert(g->getTickCount() == 2);
    assert(sim.nextEventTime() == 30.0);
}

void test5() {
    shared_ptr<genr> g = make_shared<genr>(10.0, 10);
    Simulator<char> sim(g);
    Bag<Event<char>> input;
    Event<char> event(g, 'a');
    input.push_back(event);
    sim.computeNextState(input, 5.0);
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
