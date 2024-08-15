#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


class Listener : public EventListener<int> {
  public:
    Listener() : EventListener<int>(), c(0), s(0), value(-1) {}
    void outputEvent(Event<int> x, double t) {
        c++;
        assert(value == -1 || x.value == value);
        value = x.value;
    }
    void stateChange(Atomic<int>* model, double t) { s++; }
    int c, s, value;
};

class Periodic : public Atomic<int> {
  public:
    Periodic(double p) : Atomic<int>(), p(p), c(0) {}
    double ta() {
        if (c > 10) {
            return adevs_inf<double>();
        }
        return p;
    }
    void delta_int() { c++; }
    void delta_ext(double, Bag<int> const &) {}
    void delta_conf(Bag<int> const &) {}
    void output_func(Bag<int> &yb) { yb.push_back(1); }

  private:
    double const p;
    int c;
};

class Receiver : public Atomic<int> {
  public:
    Receiver() : Atomic<int>(), c(0) {}
    double ta() { return adevs_inf<double>(); }
    void delta_int() {}
    void delta_ext(double e, Bag<int> const &xb) { c += xb.size(); }
    void delta_conf(Bag<int> const &xb) { delta_ext(0.0, xb); }
    int get_c() const { return c; }
    void output_func(Bag<int> &) {}

  private:
    int c;
};

class Trigger : public MealyAtomic<int> {
  public:
    Trigger()
        : MealyAtomic<int>(), ttg(adevs_inf<double>()), external_events(0) {}
    int external_event_count() const { return external_events; }
    double ta() { return ttg; }
    void delta_int() { ttg = adevs_inf<double>(); }
    void delta_ext(double e, Bag<int> const &xb) {
        assert(ee == e);
        external_events++;
        ttg = 1.0;
    }
    void delta_conf(Bag<int> const &xb) { ttg = 1.0; }
    void output_func(Bag<int> &yb) {
        // Turn off
        yb.push_back(0);
    }
    void output_func(Bag<int> const &xb, Bag<int> &yb) {
        // Turn on
        yb.push_back(*(xb.begin()));
    }
    void output_func(double e, Bag<int> const &xb, Bag<int> &yb) {
        ee = e;
        output_func(xb, yb);
    }

  private:
    double ttg, ee;
    int external_events;
};

void test1() {
    shared_ptr<SimpleDigraph<int>> model = make_shared<SimpleDigraph<int>>();
    shared_ptr<Trigger> trigger = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    model->add(trigger);
    model->add(periodic);
    model->couple(periodic, trigger);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        sim->execNextEvent();
        assert((l->c == 1 && l->value == 0) || (l->c == 2 && l->value == 1));
        assert(l->c == l->s);
        l->c = l->s = 0;
        l->value = -1;
    }
}

void test2() {
    shared_ptr<SimpleDigraph<int>> model = make_shared<SimpleDigraph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    model->add(triggera);
    model->add(triggerb);
    model->add(periodic);
    model->couple(periodic, triggera);
    model->couple(periodic, triggerb);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        sim->execNextEvent();
        assert((l->c == 2 && l->value == 0) || (l->c == 3 && l->value == 1));
        assert(l->c == l->s);
        l->c = l->s = 0;
        l->value = -1;
    }
}

void test3() {
    shared_ptr<SimpleDigraph<int>> model = make_shared<SimpleDigraph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    shared_ptr<Receiver> rx = make_shared<Receiver>();
    model->add(triggera);
    model->add(triggerb);
    model->add(periodic);
    model->add(rx);
    model->couple(periodic, triggera);
    model->couple(periodic, triggerb);
    model->couple(triggera, rx);
    model->couple(triggerb, rx);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        int c = rx->get_c();
        sim->execNextEvent();
        assert((l->c == 2 && l->value == 0) || (l->c == 3 && l->value == 1));
        assert(l->c == l->s - 1);
        l->c = l->s = 0;
        l->value = -1;
        assert(rx->get_c() == c + 2);
    }
}

void test4() {
    bool except = false;
    shared_ptr<SimpleDigraph<int>> model = make_shared<SimpleDigraph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    model->add(triggera);
    model->add(triggerb);
    model->add(periodic);
    model->couple(periodic, triggera);
    model->couple(triggera, triggerb);
    model->couple(triggerb, triggera);
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        try {
            sim->execNextEvent();
        } catch (...) {
            except = true;
            break;
        }
    }
    assert(except);
}

void test5() {
    cout << "TEST 5" << endl;
    shared_ptr<SimpleDigraph<int>> model = make_shared<SimpleDigraph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(1.0);
    shared_ptr<Receiver> rx = make_shared<Receiver>();
    model->add(triggera);
    model->add(triggerb);
    model->add(periodic);
    model->add(rx);
    model->couple(periodic, triggera);
    model->couple(periodic, triggerb);
    model->couple(triggera, rx);
    model->couple(triggerb, rx);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        int c = rx->get_c();
        sim->execNextEvent();
        assert(periodic->ta() == adevs_inf<double>() ||
               (l->c == 3 && l->value == 1));
        assert(l->c == l->s - 1);
        l->c = l->s = 0;
        l->value = -1;
        assert(rx->get_c() == c + 2);
    }
    cout << "TEST 5 PASSED" << endl;
}

void test6() {
    cout << "TEST 6" << endl;
    shared_ptr<SimpleDigraph<int>> model = make_shared<SimpleDigraph<int>>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(10.0);
    shared_ptr<Trigger> trigger = make_shared<Trigger>();
    model->add(periodic);
    model->add(trigger);
    model->couple(periodic, trigger);
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    while (sim->nextEventTime() < 12.0) {
        sim->execNextEvent();
    }
    assert(trigger->external_event_count() == 1);
    cout << "TEST 6 PASSED" << endl;
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    return 0;
}
