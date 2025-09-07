#include <list>
#include <memory>
#include "adevs/adevs.h"
#include "SimpleAtomic.h"

using namespace adevs;



class Structure : public Atomic<char> {
  public:
    Structure(std::shared_ptr<Graph<char>>& graph) : Atomic<char>(),graph(graph) {
        std::shared_ptr<SimpleAtomic> model = std::make_shared<SimpleAtomic>();
        graph->add_atomic(model);
    }
    void delta_int() {
        std::shared_ptr<SimpleAtomic> model = std::make_shared<SimpleAtomic>();
        graph->add_atomic(model);
    }
    void delta_ext(double, std::list<PinValue<char>> const &) { assert(false); }
    void delta_conf(std::list<PinValue<char>> const &) { assert(false); }  
    void output_func(std::list<PinValue<char>> &) {}
    double ta() { return 1.0; }

    private:
        std::shared_ptr<Graph<char>> graph;
};

int main() {
    std::shared_ptr<Graph<char>> graph = std::make_shared<Graph<char>>();
    std::shared_ptr<Structure> model = std::make_shared<Structure>(graph);
    graph->add_atomic(model);
    std::shared_ptr<Simulator<char>> sim = std::make_shared<Simulator<char>>(graph);

    while (sim->nextEventTime() < adevs_inf<double>() && SimpleAtomic::atomic_number < 10) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs + 1 == SimpleAtomic::atomic_number);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
