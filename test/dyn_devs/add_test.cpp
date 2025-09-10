#include <list>
#include <memory>
#include "adevs/adevs.h"
#include "SimpleAtomic.h"

using Simulator = adevs::Simulator<char>;
using Atomic = adevs::Atomic<char>;
using PinValue = adevs::PinValue<char>;
using Graph = adevs::Graph<char>;

class Structure : public adevs::Atomic<char> {
  public:
    Structure(std::shared_ptr<Graph>& graph) : Atomic(),graph(graph) {
        std::shared_ptr<SimpleAtomic> model = std::make_shared<SimpleAtomic>();
        graph->add_atomic(model);
    }
    void delta_int() {
        std::shared_ptr<SimpleAtomic> model = std::make_shared<SimpleAtomic>();
        graph->add_atomic(model);
    }
    void delta_ext(double, std::list<PinValue> const &) { assert(false); }
    void delta_conf(std::list<PinValue> const &) { assert(false); }
    void output_func(std::list<PinValue> &) {}
    double ta() { return 1.0; }

    private:
        std::shared_ptr<Graph> graph;
};

int main() {
    std::shared_ptr<Graph> graph = std::make_shared<Graph>();
    std::shared_ptr<Structure> model = std::make_shared<Structure>(graph);
    graph->add_atomic(model);
    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(graph);

    while (sim->nextEventTime() < adevs_inf<double>() && SimpleAtomic::atomic_number < 10) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs + 1 == SimpleAtomic::atomic_number);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
