#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"


// using namespace adevs;


class Listener : public adevs::EventListener<int> {
  public:
    Listener()
        : adevs::EventListener<int>(), output_count(0), state_changes(0), value(-1) {}
    void outputEvent(adevs::Atomic<int>&, adevs::PinValue<int>& x, double) {
        output_count++;
        assert(value == -1 || x.value == value);
        value = x.value;
    }
    void inputEvent(adevs::Atomic<int>&, adevs::PinValue<int>&, double){}
    void stateChange(adevs::Atomic<int>&, double) {
        state_changes++;
    }
    int output_count, state_changes, value;
};

class Periodic : public adevs::Atomic<int> {
  public:
    Periodic(double period)
        : adevs::Atomic<int>(), period(period), internal_transitions(0) {}
    double ta() {
        if (internal_transitions > 10) {
            return adevs_inf<double>();
        }
        return period;
    }
    void delta_int() { internal_transitions++; }
    void delta_ext(double, std::list<adevs::PinValue<int>> const &) {}
    void delta_conf(std::list<adevs::PinValue<int>> const &) {}
    void output_func(std::list<adevs::PinValue<int>> &output) {
        output.push_back(adevs::PinValue<int>(this->output,1));
    }

    const adevs::pin_t output;

  private:
    double const period;
    int internal_transitions;
};

class Receiver : public adevs::Atomic<int> {
  public:
    Receiver() : adevs::Atomic<int>(), event_count(0) {}
    double ta() { return adevs_inf<double>(); }
    void delta_int() {}
    void delta_ext(double, std::list<adevs::PinValue<int>> const &xb) {
        event_count += xb.size();
    }
    void delta_conf(std::list<adevs::PinValue<int>> const &xb) { delta_ext(0.0, xb); }
    int get_event_count() const { return event_count; }
    void output_func(std::list<adevs::PinValue<int>> &) {}

  private:
    int event_count;
};

class Trigger : public adevs::MealyAtomic<int> {
  public:
    Trigger()
        : adevs::MealyAtomic<int>(),
          time_left(adevs_inf<double>()),
          external_events(0) {}
    int external_event_count() const { return external_events; }
    double ta() { return time_left; }
    void delta_int() { time_left = adevs_inf<double>(); }
    void delta_ext(double e, std::list<adevs::PinValue<int>> const &) {
        assert(time_elapsed == e);
        external_events++;
        time_left = 1.0;
    }
    void delta_conf(std::list<adevs::PinValue<int>> const &) { time_left = 1.0; }
    void output_func(std::list<adevs::PinValue<int>> &yb) {
        // Turn off
        yb.push_back(adevs::PinValue<int>(output,0));
    }
    void confluent_output_func(std::list<adevs::PinValue<int>> const &xb, std::list<adevs::PinValue<int>> &yb) {
        // Turn on
        yb.push_back(adevs::PinValue<int>(output,(*(xb.begin())).value));
    }
    void external_output_func(double e, std::list<adevs::PinValue<int>> const &xb, std::list<adevs::PinValue<int>> &yb) {
        time_elapsed = e;
        confluent_output_func(xb, yb);
    }

    const adevs::pin_t output;
  private:
    double time_left, time_elapsed;
    int external_events;
};

void test1() {
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    std::shared_ptr<Trigger> trigger = std::make_shared<Trigger>();
    std::shared_ptr<Periodic> periodic = std::make_shared<Periodic>(sqrt(2.0));

    model->add_atomic(trigger);
    model->add_atomic(periodic);

    model->connect(periodic->output, trigger);

    std::shared_ptr<Listener> listener = std::make_shared<Listener>();
    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(model);
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
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    std::shared_ptr<Trigger> triggera = std::make_shared<Trigger>();
    std::shared_ptr<Trigger> triggerb = std::make_shared<Trigger>();
    std::shared_ptr<Periodic> periodic = std::make_shared<Periodic>(sqrt(2.0));
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->connect(periodic->output, triggera);
    model->connect(periodic->output, triggerb);
    std::shared_ptr<Listener> l = std::make_shared<Listener>();
    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(model);
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
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    std::shared_ptr<Trigger> triggera = std::make_shared<Trigger>();
    std::shared_ptr<Trigger> triggerb = std::make_shared<Trigger>();
    std::shared_ptr<Periodic> periodic = std::make_shared<Periodic>(sqrt(2.0));
    std::shared_ptr<Receiver> rx = std::make_shared<Receiver>();
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->add_atomic(rx);
    model->connect(periodic->output, triggera);
    model->connect(periodic->output, triggerb);
    model->connect(triggera->output, rx);
    model->connect(triggerb->output, rx);
    std::shared_ptr<Listener> l = std::make_shared<Listener>();
    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(model);
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
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    std::shared_ptr<Trigger> triggera = std::make_shared<Trigger>();
    std::shared_ptr<Trigger> triggerb = std::make_shared<Trigger>();
    std::shared_ptr<Periodic> periodic = std::make_shared<Periodic>(sqrt(2.0));
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->connect(periodic->output, triggera);
    model->connect(triggera->output, triggerb);
    model->connect(triggerb->output, triggera);
    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(model);
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
    std::cout << "TEST 5" << std::endl;
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    std::shared_ptr<Trigger> triggera = std::make_shared<Trigger>();
    std::shared_ptr<Trigger> triggerb = std::make_shared<Trigger>();
    std::shared_ptr<Periodic> periodic = std::make_shared<Periodic>(1.0);
    std::shared_ptr<Receiver> rx = std::make_shared<Receiver>();
    model->add_atomic(triggera);
    model->add_atomic(triggerb);
    model->add_atomic(periodic);
    model->add_atomic(rx);
    model->connect(periodic->output, triggera);
    model->connect(periodic->output, triggerb);
    model->connect(triggera->output, rx);
    model->connect(triggerb->output, rx);
    std::shared_ptr<Listener> l = std::make_shared<Listener>();
    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(model);
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
    std::cout << "TEST 5 PASSED" << std::endl;
}

void test6() {
    std::cout << "TEST 6" << std::endl;
    std::shared_ptr<adevs::Graph<int>> model = std::make_shared<adevs::Graph<int>>();
    std::shared_ptr<Periodic> periodic = std::make_shared<Periodic>(10.0);
    std::shared_ptr<Trigger> trigger = std::make_shared<Trigger>();
    model->add_atomic(periodic);
    model->add_atomic(trigger);
    model->connect(periodic->output, trigger);
    std::shared_ptr<adevs::Simulator<int>> sim = std::make_shared<adevs::Simulator<int>>(model);
    while (sim->nextEventTime() < 12.0) {
        sim->execNextEvent();
    }
    assert(trigger->external_event_count() == 1);
    std::cout << "TEST 6 PASSED" << std::endl;
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
