#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"


using PinValue = adevs::PinValue<int>;
using pin_t = adevs::pin_t;
using sd_time = adevs::sd_time<int>;
using Atomic = adevs::Atomic<int, sd_time>;
using EventListener = adevs::EventListener<int, sd_time>;
using Simulator = adevs::Simulator<int, sd_time>;
using Graph = adevs::Graph<int, sd_time>;

class Model : public Atomic {
  public:
    Model(std::string name) : Atomic(), name(name), count(0) {}
    sd_time ta() {
        if (count % 2 == 0) {
            return sd_time(1, 0);
        }
        return sd_time(0, 0);
    }
    void delta_int() { count++; }
    void delta_ext(sd_time, std::list<PinValue> const &) {}
    void delta_conf(std::list<PinValue> const &) { delta_int(); }
    void output_func(std::list<PinValue> &y) {
        PinValue yy(output_pin, count);
        y.push_back(yy);
    }

    std::string get_name() const { return name; }

    pin_t const output_pin;

  private:
    std::string const name;
    int count;
};

class Passive : public Atomic {
  public:
    Passive(std::string name) : Atomic(), name(name) {}
    sd_time ta() { return adevs_inf<sd_time>(); }
    void delta_int() {}
    void delta_ext(sd_time e, std::list<PinValue> const &) {
        std::cout << name << " " << e << std::endl;
    }
    void delta_conf(std::list<PinValue> const &) {}
    void output_func(std::list<PinValue> &) {}

    std::string get_name() const { return name; }

  private:
    std::string const name;
};

class MyEventListener : public EventListener {
  public:
    MyEventListener() : EventListener() {}
    void outputEvent(Atomic &atomic, PinValue &x, sd_time t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        std::cout << model->get_name() << " t = " << t << " , y = " << x.value << std::endl;
    }
    void stateChange(Atomic &atomic, sd_time t) {
        Model* cast_model = dynamic_cast<Model*>(&atomic);
        if (cast_model != nullptr) {
            std::cout << cast_model->get_name() << " t = " << t << " , state changed " << std::endl;
        }
        Passive* passive = dynamic_cast<Passive*>(&atomic);
        if (passive != nullptr) {
            std::cout << passive->get_name() << " t = " << t << " , state changed " << std::endl;
        }
    }
    void inputEvent(Atomic &atomic, PinValue &x, sd_time t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        if (model != nullptr) {
            std::cout << model->get_name() << " t = " << t << " , x = " << x.value << std::endl;
        }
        Passive* passive = dynamic_cast<Passive*>(&atomic);
        if (passive != nullptr) {
            std::cout << passive->get_name() << " t = " << t << " , x = " << x.value << std::endl;
        }
    }
};

void test1() {
    std::cout << "TEST 1" << std::endl;
    std::shared_ptr<Model> model = std::make_shared<Model>("model");
    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time(10, 0)) {
        sd_time tnew = sim->nextEventTime() + adevs_epsilon<sd_time>();
        assert(sim->execNextEvent() == tnew);
    }
}

void test2() {
    std::cout << "TEST 2" << std::endl;
    std::shared_ptr<Model> A = std::make_shared<Model>("A");
    std::shared_ptr<Passive> B = std::make_shared<Passive>("B");

    std::shared_ptr<Graph> model = std::make_shared<Graph>();
    model->add_atomic(A);
    model->add_atomic(B);
    model->connect(A->output_pin, B);

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);

    while (sim->nextEventTime() < sd_time(10, 0)) {
        sd_time tnew = sim->nextEventTime() + adevs_epsilon<sd_time>();
        assert(sim->execNextEvent() == tnew);
    }
}

int main() {
    test1();
    test2();
    return 0;
}
