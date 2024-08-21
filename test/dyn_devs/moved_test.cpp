#include <iostream>
#include <list>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"
using namespace adevs;
using namespace std;

class SimpleNetwork : public Network<PortValue<char>> {
  public:
    SimpleNetwork(SimpleNetwork** networks, int id)
        : Network<PortValue<char>>(), networks(networks), id(id) {
        networks[id] = this;
        if (id == 0) {
            model = make_shared<SimpleAtomic>();
            model->setParent(this);
        } else {
            model = NULL;
        }
    }

    void getComponents(set<Devs<PortValue<char>>*> &c) {
        if (model != nullptr) {
            c.insert(model.get());
        }
    }

    void route(PortValue<char> const &, Devs<PortValue<char>>*,
               Bag<Event<PortValue<char>>> &) {}

    bool model_transition() {
        if (model != nullptr) {
            int next = (id + 1) % 2;
            networks[next]->model = model;
            model->setParent(networks[next]);
            // Shouldn't need to unschedule or schedule anything.
            // The model is still active on a different network
            model = nullptr;
            return true;
        } else {
            return false;
        }
    }

  private:
    shared_ptr<Devs<PortValue<char>>> model = nullptr;
    SimpleNetwork** networks;
    int id;
};

int main() {
    shared_ptr<Digraph<char>> top_model = make_shared<Digraph<char>>();

    SimpleNetwork* networks[2];

    shared_ptr<SimpleNetwork> model0 = make_shared<SimpleNetwork>(networks, 0);
    shared_ptr<SimpleNetwork> model1 = make_shared<SimpleNetwork>(networks, 1);

    top_model->add(model0);
    top_model->add(model1);

    shared_ptr<Simulator<PortValue<char>>> sim =
        make_shared<Simulator<PortValue<char>>>(top_model);

    while (sim->nextEventTime() < 10.0) {
        assert(SimpleAtomic::atomic_number == 1);
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs == 1);
        SimpleAtomic::internal_execs = 0;
    }

    return 0;
}
