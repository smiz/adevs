#include <iostream>
#include <list>
#include <memory>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"

using Simulator = adevs::Simulator<char>;
using Atomic = adevs::Atomic<char>;
using PinValue = adevs::PinValue<char>;
using Graph = adevs::Graph<char>;

class SimpleNetwork : public Atomic {
  public:
    SimpleNetwork(std::shared_ptr<Graph> &graph) : Atomic(), graph(graph) {
        for (int i = 0; i < 10; i++) {
            std::shared_ptr<SimpleAtomic> model = std::make_shared<SimpleAtomic>();
            graph->add_atomic(model);
            models.push_back(model);
        }
    }
    void delta_int() {
        graph->remove_atomic(models.back());
        models.pop_back();
    }
    void delta_ext(double, std::list<PinValue> const &) { assert(false); }
    void delta_conf(std::list<PinValue> const &) { assert(false); }
    void output_func(std::list<PinValue> &) {}
    double ta() { return 1.0; }

  private:
    std::shared_ptr<Graph> graph;
    std::list<std::shared_ptr<Atomic>> models;
};

int main() {
    std::shared_ptr<Graph> graph = std::make_shared<Graph>();
    std::shared_ptr<SimpleNetwork> model = std::make_shared<SimpleNetwork>(graph);
    graph->add_atomic(model);
    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(graph);

    while (sim->nextEventTime() < adevs_inf<double>() && SimpleAtomic::atomic_number != 0) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs == SimpleAtomic::atomic_number + 1);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
