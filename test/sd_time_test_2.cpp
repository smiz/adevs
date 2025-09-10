#include <cassert>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"


// using namespace adevs;


class Model : public adevs::Atomic<int, adevs::sd_time<int>> {
  public:
    Model(std::string name) : adevs::Atomic<int, adevs::sd_time<int>>(), name(name), count(0) {}
    adevs::sd_time<int> ta() {
        if (count % 2 == 0) {
            return adevs::sd_time<int>(1, 0);
        }
        return adevs::sd_time<int>(0, 0);
    }
    void delta_int() { count++; }
    void delta_ext(adevs::sd_time<int>, std::list<adevs::PinValue<int>> const &) {}
    void delta_conf(std::list<adevs::PinValue<int>> const &) { delta_int(); }
    void output_func(std::list<adevs::PinValue<int>> &y) {
    	adevs::PinValue<int> yy(output_pin,count);
        y.push_back(yy);
    }

    std::string get_name() const { return name; }

    const adevs::pin_t output_pin;
  private:
    std::string const name;
    int count;
};

class Passive : public adevs::Atomic<int, adevs::sd_time<int>> {
  public:
    Passive(std::string name) : adevs::Atomic<int, adevs::sd_time<int>>(), name(name) {}
    adevs::sd_time<int> ta() { return adevs_inf<adevs::sd_time<int>>(); }
    void delta_int() {}
    void delta_ext(adevs::sd_time<int> e, std::list<adevs::PinValue<int>> const &) {
        std::cout << name << " " << e << std::endl;
    }
    void delta_conf(std::list<adevs::PinValue<int>> const &) {}
    void output_func(std::list<adevs::PinValue<int>> &) {}

    std::string get_name() const { return name; }

  private:
    std::string const name;
};

class MyEventListener : public adevs::EventListener<int, adevs::sd_time<int>> {
  public:
    MyEventListener() : adevs::EventListener<int, adevs::sd_time<int>>() {}
    void outputEvent(adevs::Atomic<int, adevs::sd_time<int>>& atomic, adevs::PinValue<int>& x, adevs::sd_time<int> t) {
        Model* model = dynamic_cast<Model*>(&atomic);
        std::cout << model->get_name() << " t = " << t << " , y = " << x.value
             << std::endl;
    }
    void stateChange(adevs::Atomic<int, adevs::sd_time<int>>& atomic, adevs::sd_time<int> t) {
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
    void inputEvent(adevs::Atomic<int, adevs::sd_time<int>>& atomic, adevs::PinValue<int>& x, adevs::sd_time<int> t) {
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
    std::shared_ptr<adevs::Simulator<int, adevs::sd_time<int>>> sim =
        std::make_shared<adevs::Simulator<int, adevs::sd_time<int>>>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);
    while (sim->nextEventTime() < adevs::sd_time<int>(10, 0)) {
        adevs::sd_time<int> tnew =
            sim->nextEventTime() + adevs_epsilon<adevs::sd_time<int>>();
        assert(sim->execNextEvent() == tnew);
    }
}

void test2() {
    std::cout << "TEST 2" << std::endl;
    std::shared_ptr<Model> A = std::make_shared<Model>("A");
    std::shared_ptr<Passive> B = std::make_shared<Passive>("B");

    std::shared_ptr<adevs::Graph<int, adevs::sd_time<int>>> model =
        std::make_shared<adevs::Graph<int, adevs::sd_time<int>>>();
    model->add_atomic(A);
    model->add_atomic(B);
    model->connect(A->output_pin, B);

    std::shared_ptr<adevs::Simulator<int, adevs::sd_time<int>>> sim =
        std::make_shared<adevs::Simulator<int, adevs::sd_time<int>>>(model);
    std::shared_ptr<MyEventListener> listener = std::make_shared<MyEventListener>();
    sim->addEventListener(listener);

    while (sim->nextEventTime() < adevs::sd_time<int>(10, 0)) {
        adevs::sd_time<int> tnew =
            sim->nextEventTime() + adevs_epsilon<adevs::sd_time<int>>();
        assert(sim->execNextEvent() == tnew);
    }
}

int main() {
    test1();
    test2();
    return 0;
}
