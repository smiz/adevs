#include <iostream>
#include <list>
#include "adevs/adevs.h"
using namespace adevs;


class Model;
using ModelPtr = std::shared_ptr<Model>;

class Model : public Atomic<ModelPtr> {
  public:
    Model(bool active, std::shared_ptr<Graph<ModelPtr>>& g) : Atomic<ModelPtr>(),g(g),self(nullptr),active(active) { num_models++; }

    void delta_int() {
        g->remove_atomic(self);
        self = nullptr;
    }

    void delta_ext(double, const std::list<PinValue<ModelPtr>>& xb) {
        assert(!active);
        std::shared_ptr<Model> m = std::make_shared<Model>(false,g);
        m->self_ptr(m);
        g->add_atomic(m);
        g->connect(m->in, m);
        g->connect(out,m->in);
        g->disconnect(xb.front().value->out, in);
        active = true;
        num_externals++;
    }
    void delta_conf(const std::list<PinValue<ModelPtr>>&) { assert(false); }
    
    void output_func(std::list<PinValue<ModelPtr>>& yb) {
        yb.push_back(PinValue<ModelPtr>(out, self));
    }
    double ta() { return (active) ? 1.0 : adevs_inf<double>(); }
    void self_ptr(ModelPtr self) { this->self = self; }

    ~Model() { num_models--; }

    pin_t in, out;
    static int num_models, num_externals;
  private:
    std::shared_ptr<Graph<ModelPtr>> g;
    ModelPtr self;
    bool active;
};

int Model::num_models = 0;
int Model::num_externals = 0;

int main() {
    int count = 0;
    std::shared_ptr<Graph<ModelPtr>> g = std::make_shared<Graph<ModelPtr>>();
    std::shared_ptr<Model> m = std::make_shared<Model>(true,g);
    m->self_ptr(m);
    g->add_atomic(m);

    std::shared_ptr<Simulator<ModelPtr>> sim =
        std::make_shared<Simulator<ModelPtr>>(g);

    while (sim->nextEventTime() < 10.0) {
        sim->execNextEvent();
        assert(Model::num_models <= 2);
        assert(Model::num_externals == count);
        count++;
    }

    return 0;
}
