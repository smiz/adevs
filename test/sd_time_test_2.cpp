#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"


using namespace adevs;


class Model : public Atomic<int, sd_time<int>> {
  public:
    Model(std::string name) : Atomic<int, sd_time<int>>(), name(name), count(0) {}
    sd_time<int> ta() {
        if (count % 2 == 0) {
            return sd_time<int>(1, 0);
        }
        return sd_time<int>(0, 0);
    }
    void delta_int() { count++; }
    void delta_ext(sd_time<int>, std::list<PinValue<int>> const &) {}
    void delta_conf(std::list<PinValue<int>> const &) { delta_int(); }
    void output_func(std::list<PinValue<int>> &y) {
        PinValue<int> yy(output_pin,count);
        y.push_back(yy);
    }

    std::string get_name() const { return name; }

    const pin_t output_pin;
  private:
    std::string const name;
    int count;
};

class Passive : public Atomic<int, sd_time<int>> {
  public:
    Passive(std::string name) : Atomic<int, sd_time<int>>(), name(name) {}
    sd_time<int> ta() { return adevs_inf<sd_time<int>>(); }
    void delta_int() {}
    void delta_ext(sd_time<int> e, std::list<PinValue<int>> const &) {
        std::cout << name << " " << e << std::endl;
    }
    void delta_conf(std::list<PinValue<int>> const &) {}
    void output_func(std::list<PinValue<int>> &) {}

    std::string get_name() const { return name; }

  private:
    std::string const name;
};

class MyEventListener : public EventListener<int, sd_time<int>> {
  public:
    MyEventListener() : EventListener<int, sd_time<int>>() {}
    void outputEvent(Atomic<int, sd_time<int>>& atomic, PinValue<int>& x, sd_time<int> t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        std::cout << model->get_name() << " t = " << t << " , y = " << x.value
             << std::endl;
    }
    void stateChange(Atomic<int, sd_time<int>>& atomic, sd_time<int> t) {
        Model* cast_model = dynamic_cast<Model*>(&atomic);
        if (cast_model != nullptr) {
            std::cout << cast_model->get_name() << " t = " << t
                 << " , state changed " << std::endl;
        }
        Passive* passive = dynamic_cast<Passive*>(&atomic);
        if (passive != nullptr) {
            std::cout << passive->get_name() << " t = " << t << " , state changed "
                 << std::endl;
        }
    }
    void inputEvent(Atomic<int, sd_time<int>>& atomic, PinValue<int>& x, sd_time<int> t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        if (model != nullptr) {
            std::cout << model->get_name() << " t = " << t << " , x = " << x.value
                 << std::endl;
        }
        Passive* passive = dynamic_cast<Passive*>(&atomic);
        if (passive != nullptr) {
            std::cout << passive->get_name() << " t = " << t << " , x = " << x.value
                 << std::endl;
        }
    }
};

void test1() {
    std::cout << "TEST 1" << std::endl;
    std::shared_ptr<Model> model = std::make_shared<Model>("model");
    std::shared_ptr<Simulator<int, sd_time<int>>> sim =
        std::make_shared<Simulator<int, sd_time<int>>>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < sd_time<int>(10, 0)) {
        sd_time<int> tnew =
            sim->nextEventTime() + adevs_epsilon<sd_time<int>>();
        assert(sim->execNextEvent() == tnew);
    }
}

void test2() {
    std::cout << "TEST 2" << std::endl;
    std::shared_ptr<Model> A = std::make_shared<Model>("A");
    std::shared_ptr<Passive> B = std::make_shared<Passive>("B");

    std::shared_ptr<Graph<int, sd_time<int>>> model =
        std::make_shared<Graph<int, sd_time<int>>>();
    model->add_atomic(A);
    model->add_atomic(B);
    model->connect(A->output_pin, B);

    std::shared_ptr<Simulator<int, sd_time<int>>> sim =
        std::make_shared<Simulator<int, sd_time<int>>>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
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
