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
    void delta_ext(double_fcmp, Bag<char> const &) { sigma = DBL_MAX; }
    void delta_conf(Bag<char> const &) { sigma = DBL_MAX; }
    void output_func(Bag<char> &y) { y.push_back('a'); }
    void gc_output(Bag<char> &g) {
        // assert(g.count('a') > 0);
    }
    ~genr() {}
    int getTickCount() { return count; }

  private:
    int ticks;
    int count;
    double_fcmp sigma;
};

void test1() {
    genr* g = new genr(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
    delete g;
}

void test2() {
    genr* g = new genr(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
    delete g;
}

class MyEventListener : public EventListener<char, double_fcmp> {
  public:
    MyEventListener() {
        count = 0;
        t_last = 0.0;
    }
    void outputEvent(Event<char, double_fcmp> x, double_fcmp t) {
        count++;
        t_last = t;
    }
    int count;
    double_fcmp t_last;
};

void test3() {
    genr* g = new genr(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    MyEventListener listener;
    sim.addEventListener(&listener);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.computeNextOutput();
        assert(listener.t_last == sim.nextEventTime());
        sim.execNextEvent();
    }
    assert(listener.count == 10);
    assert(g->getTickCount() == 10);
    delete g;
}

void test4() {
    genr* g = new genr(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    Bag<Event<char, double_fcmp>> input;
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
    delete g;
}

void test5() {
    genr* g = new genr(10.0, 10);
    Simulator<char, double_fcmp> sim(g);
    Bag<Event<char, double_fcmp>> input;
    Event<char, double_fcmp> event(g, 'a');
    input.push_back(event);
    sim.computeNextState(input, 5.0);
    assert(sim.nextEventTime() == DBL_MAX);
    assert(g->getTickCount() == 0);
    delete g;
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}
