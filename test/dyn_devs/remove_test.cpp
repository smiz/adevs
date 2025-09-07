#include <iostream>
#include <list>
#include <memory>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"

using namespace adevs;



class SimpleNetwork : public Atomic<char> {
  public:
    SimpleNetwork(std::shared_ptr<Graph<char>>& graph) : Atomic<char>(),graph(graph) {
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
    void delta_ext(double, std::list<PinValue<char>> const &) { assert(false); }
    void delta_conf(std::list<PinValue<char>> const &) { assert(false); }
    void output_func(std::list<PinValue<char>> &) {}
    double ta() { return 1.0; }

  private:
    std::shared_ptr<Graph<char>> graph;
    std::list<std::shared_ptr<Atomic<char>>> models;
};

int main() {
    std::shared_ptr<Graph<char>> graph = std::make_shared<Graph<char>>();
    std::shared_ptr<SimpleNetwork> model = std::make_shared<SimpleNetwork>(graph);
    graph->add_atomic(model);
    std::shared_ptr<Simulator<char>> sim = std::make_shared<Simulator<char>>(graph);

    while (sim->nextEventTime() < adevs_inf<double>() && SimpleAtomic::atomic_number != 0) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs == SimpleAtomic::atomic_number + 1);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
