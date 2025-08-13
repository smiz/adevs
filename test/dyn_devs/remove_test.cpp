#include <iostream>
#include <list>
#include <memory>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"

using namespace adevs;
using namespace std;


class SimpleNetwork : public Atomic<char> {
  public:
    SimpleNetwork(shared_ptr<Graph<char>>& graph) : Atomic<char>(),graph(graph) {
        for (int i = 0; i < 10; i++) {
            shared_ptr<SimpleAtomic> model = make_shared<SimpleAtomic>();
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
    shared_ptr<Graph<char>> graph;
    list<shared_ptr<Atomic<char>>> models;
};

int main() {
    shared_ptr<Graph<char>> graph = make_shared<Graph<char>>();
    shared_ptr<SimpleNetwork> model = make_shared<SimpleNetwork>(graph);
    graph->add_atomic(model);
    shared_ptr<Simulator<char>> sim = make_shared<Simulator<char>>(graph);

    while (sim->nextEventTime() < adevs_inf<double>() && SimpleAtomic::atomic_number != 0) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs == SimpleAtomic::atomic_number + 1);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
