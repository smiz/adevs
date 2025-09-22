#include <cassert>
#include <iostream>
#include "adevs/models.h"
#include "adevs/simulator.h"


using pin_t = adevs::pin_t;
using Atomic = adevs::Atomic<char>;
using PinValue = adevs::PinValue<char>;
using Simulator = adevs::Simulator<char>;
using EventListener = adevs::EventListener<char>;
using Graph = adevs::Graph<char>;

class genr : public Atomic {
  public:
    genr(double period, int ticks) : Atomic(), ticks(ticks), count(0), sigma(period) {}
    double ta() { return sigma; }
    void delta_int() {
        count++;
        assert(count <= ticks);
        if (count == ticks) {
            sigma = adevs_inf<double>();
        }
    }
    void delta_ext(double, std::list<PinValue> const &) { sigma = adevs_inf<double>(); }
    void delta_conf(std::list<PinValue> const &) { sigma = adevs_inf<double>(); }
    void output_func(std::list<PinValue> &y) {
        PinValue output(output_pin, 'a');
        y.push_back(output);
    }
    int getTickCount() { return count; }

    pin_t const output_pin;

  private:
    int ticks;
    int count;
    double sigma;
};

void test1() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator sim(g);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }
    assert(g->getTickCount() == 10);
}

void test2() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator sim(g);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.computeNextOutput();
        sim.computeNextState();
    }
    assert(g->getTickCount() == 10);
}

class MyEventListener : public EventListener {
  public:
    MyEventListener() {
        count = 0;
        t_last = 0.0;
    }
    void outputEvent(Atomic &, PinValue &, double t) {
        count++;
        t_last = t;
    }
    void inputEvent(Atomic &, PinValue &, double) {}
    void stateChange(Atomic &, double) {}
    int count;
    double t_last;
};

void test3() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator sim(g);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim.addEventListener(listener);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.computeNextOutput();
        assert(listener->t_last == sim.nextEventTime());
        sim.computeNextState();
    }
    assert(listener->count == 10);
    assert(g->getTickCount() == 10);
}

void test4() {
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    Simulator sim(g);
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
    pin_t input_pin;
    PinValue input(input_pin, 'a');
    std::shared_ptr<genr> g = std::make_shared<genr>(10.0, 10);
    std::shared_ptr<Graph> graph = std::make_shared<Graph>();
    graph->add_atomic(g);
    graph->connect(input_pin, g);
    Simulator sim(graph);
    sim.setNextTime(5.0);
    assert(sim.nextEventTime() == 5.0);
    sim.injectInput(input);
    sim.computeNextOutput();
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
