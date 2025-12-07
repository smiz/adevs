#include <random>
#include "adevs/adevs.h"
#include "adevs/parallel.h"

using Bag = std::list<adevs::PinValue<unsigned>>;
using Time = adevs::sd_time<int>;

class Model: public adevs::Atomic<unsigned,Time> {
    public:
    Model(unsigned seed):adevs::Atomic<unsigned,Time>(),uniform(0,10) {
        q.count = 0;
        q.largest = 0;
        q.rand.seed(seed);
        q.remaining_time = Time(uniform(q.rand),1);
    }
    void delta_int() {
        q.remaining_time = Time(uniform(q.rand),1);
    }
    void delta_ext(Time e, const Bag& x) {
        q.remaining_time -= e;
        q.count += x.size();
        for (auto input: x) {
            q.largest = std::max(q.largest,input.value);
        }
    }
    void delta_conf(const Bag& x) {
        q.count += x.size();
        for (auto input: x) {
            q.largest = std::max(q.largest,input.value);
        }
        q.remaining_time = Time(uniform(q.rand),1);
    }
    void output_func(Bag& y) {
        adevs::PinValue<unsigned> event(output,q.count);
        y.push_back(event);
    }
    Time ta() { return q.remaining_time; }
    void* make_checkpoint() { return new state_t(q); }
    void destroy_checkpoint(void* checkpoint) {
        delete static_cast<state_t*>(checkpoint);
    }
    void restore_checkpoint(void* checkpoint) {
        q = *(static_cast<state_t*>(checkpoint));
    }

    adevs::pin_t output;

    unsigned get_count() const { return q.count; }
    unsigned get_largest() const { return q.largest; }
    Time get_remaining() const { return q.remaining_time; }

    private:
    std::uniform_int_distribution<int> uniform;
    struct state_t {
        unsigned count;
        unsigned largest;
        Time remaining_time;
        std::mt19937 rand;
    };
    state_t q;
};

void test1() {
    // Sequential simulation
    auto a = std::make_shared<Model>(0);
    auto b = std::make_shared<Model>(1);
    auto graph = std::make_shared<adevs::Graph<unsigned,Time>>();
    graph->add_atomic(a);
    graph->add_atomic(b);
    graph->connect(a->output,b);
    graph->connect(b->output,a);
    adevs::Simulator<unsigned,Time> sim(graph);
    while (sim.nextEventTime() < Time(100,0)) {
        sim.execNextEvent();
    }
    // Parallel simulation
    auto pa = std::make_shared<Model>(0);
    auto pb = std::make_shared<Model>(1);
    auto pgraph = std::make_shared<adevs::Graph<unsigned,Time>>();
    pgraph->add_atomic(pa);
    pgraph->add_atomic(pb);
    pgraph->connect(pa->output,pb);
    pgraph->connect(pb->output,pa);
    adevs::ParallelSimulator<unsigned,Time> psim(pgraph);
    psim.exec_until(Time(100,0));

    std::cout << a->get_count() << " = " << pa->get_count() << std::endl;
    std::cout << a->get_largest() << " = " << pa->get_largest() << std::endl;
    std::cout << a->get_remaining() << " = " << pa->get_remaining() << std::endl;
}

int main() {
    test1();
    return 0;
}