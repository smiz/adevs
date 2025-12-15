#include <random>
#include <omp.h>
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

void test1(unsigned seed[4]) {
    // Sequential simulation
    auto a = std::make_shared<Model>(seed[0]);
    auto b = std::make_shared<Model>(seed[1]);
    auto c = std::make_shared<Model>(seed[2]);
    auto d = std::make_shared<Model>(seed[3]);
    auto graph = std::make_shared<adevs::Graph<unsigned,Time>>();
    graph->add_atomic(a);
    graph->add_atomic(b);
    graph->add_atomic(c);
    graph->add_atomic(d);
    graph->connect(c->output,a);
    graph->connect(a->output,b);
    graph->connect(b->output,a);
    graph->connect(b->output,d);
    adevs::Simulator<unsigned,Time> sim(graph);
    while (sim.nextEventTime() < Time(100,0)) {
        sim.execNextEvent();
    }
    // Parallel simulation
    auto pa = std::make_shared<Model>(seed[0]);
    auto pb = std::make_shared<Model>(seed[1]);
    auto pc = std::make_shared<Model>(seed[2]);
    auto pd = std::make_shared<Model>(seed[3]);
    auto pgraph = std::make_shared<adevs::Graph<unsigned,Time>>();
    pgraph->add_atomic(pa);
    pgraph->add_atomic(pb);
    pgraph->add_atomic(pc);
    pgraph->add_atomic(pd);
    pgraph->connect(pc->output,pa);
    pgraph->connect(pa->output,pb);
    pgraph->connect(pb->output,pa);
    pgraph->connect(pb->output,pd);
    adevs::ParallelSimulator<unsigned,Time> psim(pgraph);
    psim.exec_until(Time(100,0));

    std::cout << a->get_count() << " = " << pa->get_count() << std::endl;
    std::cout << a->get_largest() << " = " << pa->get_largest() << std::endl;
    std::cout << a->get_remaining() << " = " << pa->get_remaining() << std::endl;
    std::cout << b->get_count() << " = " << pb->get_count() << std::endl;
    std::cout << b->get_largest() << " = " << pb->get_largest() << std::endl;
    std::cout << b->get_remaining() << " = " << pb->get_remaining() << std::endl;
    std::cout << c->get_count() << " = " << pc->get_count() << std::endl;
    std::cout << c->get_largest() << " = " << pc->get_largest() << std::endl;
    std::cout << c->get_remaining() << " = " << pc->get_remaining() << std::endl;
    std::cout << d->get_count() << " = " << pd->get_count() << std::endl;
    std::cout << d->get_largest() << " = " << pd->get_largest() << std::endl;
    std::cout << d->get_remaining() << " = " << pd->get_remaining() << std::endl;

    assert(a->get_count() == pa->get_count());
    assert(b->get_count() == pb->get_count());
    assert(c->get_count() == pc->get_count());
    assert(d->get_count() == pd->get_count());
    assert(a->get_largest() == pa->get_largest());
    assert(b->get_largest() == pb->get_largest());
    assert(c->get_largest() == pc->get_largest());
    assert(d->get_largest() == pd->get_largest());
    assert(a->get_remaining() == pa->get_remaining());
    assert(b->get_remaining() == pb->get_remaining());
    assert(c->get_remaining() == pc->get_remaining());
    assert(d->get_remaining() == pd->get_remaining());
}

int main() {
    unsigned seed[4];
    for (unsigned i = 0; i < 1000; i++) {
        for (unsigned j = 0; j < 4; j++) {
            seed[j] = rand();
        }
        test1(seed);
    }
    std::cout << "Finished with " << omp_get_max_threads() << " threads" << std::endl;
    return 0;
}