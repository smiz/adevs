#include <iostream>
#include <list>
#include "adevs/adevs.h"

class Model;
using ModelPtr = std::shared_ptr<Model>;

using Atomic = adevs::Atomic<ModelPtr>;
using Graph = adevs::Graph<ModelPtr>;
using PinValue = adevs::PinValue<ModelPtr>;
using Simulator = adevs::Simulator<ModelPtr>;
using pin_t = adevs::pin_t;

class Model : public Atomic {
  public:
    Model(bool active, std::shared_ptr<Graph> &g) : Atomic(), g(g), self(nullptr), active(active) {
        num_models++;
    }

    void delta_int() {
        g->remove_atomic(self);
        self = nullptr;
    }

    void delta_ext(double, std::list<PinValue> const &xb) {
        assert(!active);
        std::shared_ptr<Model> m = std::make_shared<Model>(false, g);
        m->self_ptr(m);
        g->add_atomic(m);
        g->connect(m->in, m);
        g->connect(out, m->in);
        g->disconnect(xb.front().value->out, in);
        active = true;
        num_externals++;
    }
    void delta_conf(std::list<PinValue> const &) { assert(false); }

    void output_func(std::list<PinValue> &yb) { yb.push_back(PinValue(out, self)); }
    double ta() { return (active) ? 1.0 : adevs_inf<double>(); }
    void self_ptr(ModelPtr self) { this->self = self; }

    ~Model() { num_models--; }

    pin_t in, out;
    static int num_models, num_externals;

  private:
    std::shared_ptr<Graph> g;
    ModelPtr self;
    bool active;
};

int Model::num_models = 0;
int Model::num_externals = 0;

int main() {
    int count = 0;
    std::shared_ptr<Graph> g = std::make_shared<Graph>();
    std::shared_ptr<Model> m = std::make_shared<Model>(true, g);
    m->self_ptr(m);
    g->add_atomic(m);

    std::shared_ptr<Simulator> sim = std::make_shared<Simulator>(g);

    while (sim->nextEventTime() < 10.0) {
        sim->execNextEvent();
        assert(Model::num_models <= 2);
        assert(Model::num_externals == count);
        count++;
    }

    return 0;
}
