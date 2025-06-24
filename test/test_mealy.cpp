#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"

using namespace std;
using namespace adevs;


class Listener : public EventListener<int> {
  public:
    Listener()
        : EventListener<int>(), output_count(0), state_changes(0), value(-1) {}
    void outputEvent(Atomic<int>&, PinValue<int>& x, double) {
        output_count++;
        assert(value == -1 || x.value == value);
        value = x.value;
    }
    void inputEvent(Atomic<int>&, PinValue<int>&, double){}
    void stateChange(Atomic<int>&, double) {
        state_changes++;
    }
    int output_count, state_changes, value;
};

class Periodic : public Atomic<int> {
  public:
    Periodic(double period)
        : Atomic<int>(), period(period), internal_transitions(0) {}
    double ta() {
        if (internal_transitions > 10) {
            return adevs_inf<double>();
        }
        return period;
    }
    void delta_int() { internal_transitions++; }
    void delta_ext(double, list<PinValue<int>> const &) {}
    void delta_conf(list<PinValue<int>> const &) {}
    void output_func(list<PinValue<int>> &output) {
        output.push_back(PinValue<int>(this->output,1));
    }

    const pin_t output;

  private:
    double const period;
    int internal_transitions;
};

class Receiver : public Atomic<int> {
  public:
    Receiver() : Atomic<int>(), event_count(0) {}
    double ta() { return adevs_inf<double>(); }
    void delta_int() {}
    void delta_ext(double, list<PinValue<int>> const &xb) {
        event_count += xb.size();
    }
    void delta_conf(list<PinValue<int>> const &xb) { delta_ext(0.0, xb); }
    int get_event_count() const { return event_count; }
    void output_func(list<PinValue<int>> &) {}

  private:
    int event_count;
};

class Trigger : public MealyAtomic<int> {
  public:
    Trigger()
        : MealyAtomic<int>(),
          time_left(adevs_inf<double>()),
          external_events(0) {}
    int external_event_count() const { return external_events; }
    double ta() { return time_left; }
    void delta_int() { time_left = adevs_inf<double>(); }
    void delta_ext(double e, list<PinValue<int>> const &) {
        assert(time_elapsed == e);
        external_events++;
        time_left = 1.0;
    }
    void delta_conf(list<PinValue<int>> const &) { time_left = 1.0; }
    void output_func(list<PinValue<int>> &yb) {
        // Turn off
        yb.push_back(PinValue<int>(output,0));
    }
    void confluent_output_func(list<PinValue<int>> const &xb, list<PinValue<int>> &yb) {
        // Turn on
        yb.push_back(PinValue<int>(output,(*(xb.begin())).value));
    }
    void external_output_func(double e, list<PinValue<int>> const &xb, list<PinValue<int>> &yb) {
        time_elapsed = e;
        confluent_output_func(xb, yb);
    }

    const pin_t output;
  private:
    double time_left, time_elapsed;
    int external_events;
};

void test1() {
    shared_ptr<Graph<int>> model = make_shared<Graph<int>>();
    shared_ptr<Trigger> trigger = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));

    model->add_atomic(trigger);
    model->add_atomic(periodic);

    model->connect(periodic->output, trigger);

    shared_ptr<Listener> listener = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(listener);

    while (sim->nextEventTime() < adevs_inf<double>()) {
        sim->execNextEvent();
        assert((listener->output_count == 1 && listener->value == 0) ||
               (listener->output_count == 2 && listener->value == 1));
        assert(listener->output_count == listener->state_changes);
        listener->output_count = listener->state_changes = 0;
        listener->value = -1;
    }
}

void test2() {
    shared_ptr<Graph<int>> model = make_shared<Graph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->connect(periodic->output, triggera);
    model->connect(periodic->output, triggerb);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        sim->execNextEvent();
        assert((l->output_count == 2 && l->value == 0) ||
               (l->output_count == 3 && l->value == 1));
        assert(l->output_count == l->state_changes);
        l->output_count = l->state_changes = 0;
        l->value = -1;
    }
}

void test3() {
    shared_ptr<Graph<int>> model = make_shared<Graph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    shared_ptr<Receiver> rx = make_shared<Receiver>();
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->add_atomic(rx);
    model->connect(periodic->output, triggera);
    model->connect(periodic->output, triggerb);
    model->connect(triggera->output, rx);
    model->connect(triggerb->output, rx);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        int c = rx->get_event_count();
        sim->execNextEvent();
        assert((l->output_count == 2 && l->value == 0) ||
               (l->output_count == 3 && l->value == 1));
        assert(l->output_count == l->state_changes - 1);
        l->output_count = l->state_changes = 0;
        l->value = -1;
        assert(rx->get_event_count() == c + 2);
    }
}

void test4() {
    bool except = false;
    shared_ptr<Graph<int>> model = make_shared<Graph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(sqrt(2.0));
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->connect(periodic->output, triggera);
    model->connect(triggera->output, triggerb);
    model->connect(triggerb->output, triggera);
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
    shared_ptr<Graph<int>> model = make_shared<Graph<int>>();
    shared_ptr<Trigger> triggera = make_shared<Trigger>();
    shared_ptr<Trigger> triggerb = make_shared<Trigger>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(1.0);
    shared_ptr<Receiver> rx = make_shared<Receiver>();
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->add_atomic(rx);
    model->connect(periodic->output, triggera);
    model->connect(periodic->output, triggerb);
    model->connect(triggera->output, rx);
    model->connect(triggerb->output, rx);
    shared_ptr<Listener> l = make_shared<Listener>();
    shared_ptr<Simulator<int>> sim = make_shared<Simulator<int>>(model);
    sim->addEventListener(l);
    while (sim->nextEventTime() < adevs_inf<double>()) {
        int c = rx->get_event_count();
        sim->execNextEvent();
        assert(periodic->ta() == adevs_inf<double>() ||
               (l->output_count == 3 && l->value == 1));
        assert(l->output_count == l->state_changes - 1);
        l->output_count = l->state_changes = 0;
        l->value = -1;
        assert(rx->get_event_count() == c + 2);
    }
    cout << "TEST 5 PASSED" << endl;
}

void test6() {
    cout << "TEST 6" << endl;
    shared_ptr<Graph<int>> model = make_shared<Graph<int>>();
    shared_ptr<Periodic> periodic = make_shared<Periodic>(10.0);
    shared_ptr<Trigger> trigger = make_shared<Trigger>();
    model->add_atomic(periodic);
    model->add_atomic(trigger);
    model->connect(periodic->output, trigger);
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
