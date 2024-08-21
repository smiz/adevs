#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


class Incr : public Atomic<int, sd_time<double>> {
  public:
    Incr() : Atomic<int, sd_time<double>>(), count(0) {}
    sd_time<double> ta() {
        if (count % 2 == 0) {
            return sd_time<double>(count, 0);
        }
        return sd_time<double>(0, count);
    }
    void delta_int() { count++; }
    void delta_ext(sd_time<double>, Bag<int> const &) {}
    void delta_conf(Bag<int> const &) {}
    void output_func(Bag<int> &y) { y.push_back(count); }
    int get_q() const { return count; }

  private:
    int count;
};

class Watch : public Atomic<int, sd_time<double>> {
  public:
    Watch() : Atomic<int, sd_time<double>>() {}
    sd_time<double> ta() { return adevs_inf<sd_time<double>>(); }
    void delta_int() { assert(false); }
    void delta_ext(sd_time<double> e, Bag<int> const &xb) {
        int count = *(xb.begin());
        sd_time<double> expect;
        if (count % 2 == 0) {
            expect = sd_time<double>(count, 0);
        } else {
            expect = sd_time<double>(0, count);
        }
        assert(e == expect);
    }
    void delta_conf(Bag<int> const &) { assert(false); }
    void output_func(Bag<int> &y) { assert(false); }
};

class MyEventListener : public EventListener<int, sd_time<double>> {
  public:
    MyEventListener() : EventListener<int, sd_time<double>>() {}
    void outputEvent(Event<int, sd_time<double>> x, sd_time<double> t) {
        cout << "t = " << t << " , y = " << x.value << endl;
    }
    void stateChange(Atomic<int, sd_time<double>>* model, sd_time<double> t) {
        Incr* incr = dynamic_cast<Incr*>(model);
        if (incr != nullptr) {
            cout << "t = " << t << " , q = " << incr->get_q()
                 << " , ta = " << incr->ta() << endl;
        } else {
            cout << "t = " << t << " , external event"
                 << " , ta = " << model->ta() << endl;
        }
    }
};

void test0() {
    cout << "TEST 0" << endl;
    assert(sd_time<>(0.0, 0) + sd_time<>(0.0, 0) == sd_time<>(0.0, 0));
    assert(sd_time<>(0, 0) + sd_time<>(1.0, -1) == sd_time<>(1.0, -1));
    assert(sd_time<>(1, 0) + sd_time<>(1, -1) == sd_time<>(2, -1));
    assert(sd_time<>(1, 1) + sd_time<>(1, -1) == sd_time<>(2, -1));
    assert(sd_time<>(1, 1) + sd_time<>(0, 4) == sd_time<>(1, 5));
    cout << "TEST 0 PASSED" << endl;
}

void test1() {
    cout << "TEST 1" << endl;
    shared_ptr<Incr> model = make_shared<Incr>();
    shared_ptr<Simulator<int, sd_time<>>> sim =
        make_shared<Simulator<int, sd_time<>>>(model);
    shared_ptr<MyEventListener> listener = make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time<>(10.0, 0)) {
        sim->execNextEvent();
    }
    cout << "TEST 1 PASSED" << endl;
}

void test2() {
    cout << "TEST 2" << endl;
    shared_ptr<Incr> a = make_shared<Incr>();
    shared_ptr<Watch> b = make_shared<Watch>();

    shared_ptr<SimpleDigraph<int, sd_time<>>> model =
        make_shared<SimpleDigraph<int, sd_time<>>>();
    model->add(a);
    model->add(b);
    model->couple(a, b);

    shared_ptr<Simulator<int, sd_time<>>> sim =
        make_shared<Simulator<int, sd_time<>>>(model);
    shared_ptr<MyEventListener> listener = make_shared<MyEventListener>();

    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time<>(10.0, 0)) {
        sim->execNextEvent();
    }
    cout << "TEST 2 PASSED" << endl;
}

int main() {
    test0();
    test1();
    test2();
    return 0;
}
