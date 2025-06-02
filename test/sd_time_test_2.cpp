#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


class Model : public Atomic<int, sd_time<int>> {
  public:
    Model(string name) : Atomic<int, sd_time<int>>(), name(name), count(0) {}
    sd_time<int> ta() {
        if (count % 2 == 0) {
            return sd_time<int>(1, 0);
        }
        return sd_time<int>(0, 0);
    }
    void delta_int() { count++; }
    void delta_ext(sd_time<int>, list<PinValue<int>> const &) {}
    void delta_conf(list<PinValue<int>> const &) { delta_int(); }
    void output_func(list<PinValue<int>> &y) {
        PinValue<int> yy(output_pin,count);
        y.push_back(yy);
    }

    string get_name() const { return name; }

    const pin_t output_pin;
  private:
    string const name;
    int count;
};

class Passive : public Atomic<int, sd_time<int>> {
  public:
    Passive(string name) : Atomic<int, sd_time<int>>(), name(name) {}
    sd_time<int> ta() { return adevs_inf<sd_time<int>>(); }
    void delta_int() {}
    void delta_ext(sd_time<int> e, list<PinValue<int>> const &) {
        cout << name << " " << e << endl;
    }
    void delta_conf(list<PinValue<int>> const &) {}
    void output_func(list<PinValue<int>> &) {}

    string get_name() const { return name; }

  private:
    string const name;
};

class MyEventListener : public EventListener<int, sd_time<int>> {
  public:
    MyEventListener() : EventListener<int, sd_time<int>>() {}
    void outputEvent(Atomic<int, sd_time<int>>& atomic, PinValue<int>& x, sd_time<int> t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        cout << model->get_name() << " t = " << t << " , y = " << x.value
             << endl;
    }
    void stateChange(Atomic<int, sd_time<int>>& atomic, sd_time<int> t) {
        Model* cast_model = dynamic_cast<Model*>(&atomic);
        if (cast_model != nullptr) {
            cout << cast_model->get_name() << " t = " << t
                 << " , state changed " << endl;
        }
        Passive* passive = dynamic_cast<Passive*>(&atomic);
        if (passive != nullptr) {
            cout << passive->get_name() << " t = " << t << " , state changed "
                 << endl;
        }
    }
    void inputEvent(Atomic<int, sd_time<int>>& atomic, PinValue<int>& x, sd_time<int> t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        if (model != nullptr) {
            cout << model->get_name() << " t = " << t << " , x = " << x.value
                 << endl;
        }
        Passive* passive = dynamic_cast<Passive*>(&atomic);
        if (passive != nullptr) {
            cout << passive->get_name() << " t = " << t << " , x = " << x.value
                 << endl;
        }
    }
};

void test1() {
    cout << "TEST 1" << endl;
    shared_ptr<Model> model = make_shared<Model>("model");
    shared_ptr<Simulator<int, sd_time<int>>> sim =
        make_shared<Simulator<int, sd_time<int>>>(model);
    shared_ptr<MyEventListener> listener = make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time<int>(10, 0)) {
        sd_time<int> tnew =
            sim->nextEventTime() + adevs_epsilon<sd_time<int>>();
        assert(sim->execNextEvent() == tnew);
    }
}

void test2() {
    cout << "TEST 2" << endl;
    shared_ptr<Model> A = make_shared<Model>("A");
    shared_ptr<Passive> B = make_shared<Passive>("B");

    shared_ptr<Graph<int, sd_time<int>>> model =
        make_shared<Graph<int, sd_time<int>>>();
    model->add_atomic(A);
    model->add_atomic(B);
    model->connect(A->output_pin, B);

    shared_ptr<Simulator<int, sd_time<int>>> sim =
        make_shared<Simulator<int, sd_time<int>>>(model);
    shared_ptr<MyEventListener> listener = make_shared<MyEventListener>();
    sim->addEventListener(listener);

    while (sim->nextEventTime() < sd_time<int>(10, 0)) {
        sd_time<int> tnew =
            sim->nextEventTime() + adevs_epsilon<sd_time<int>>();
        assert(sim->execNextEvent() == tnew);
    }
}

int main() {
    test1();
    test2();
    return 0;
}
