#include <iostream>
#include <list>
#include <memory>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"

using namespace adevs;
using namespace std;


class SimpleNetwork : public Network<SimpleIO> {
  public:
    SimpleNetwork() : Network<SimpleIO>() {
        shared_ptr<SimpleAtomic> model = make_shared<SimpleAtomic>();
        model->setParent(this);
        models.push_back(model);
    }

    void getComponents(set<Devs<SimpleIO>*> &c) {
        for (auto iter : models) {
            c.insert(iter.get());
        }
    }

    void route(SimpleIO const &, Devs<SimpleIO>*, Bag<Event<SimpleIO>> &) {}

    bool model_transition() {
        shared_ptr<SimpleAtomic> model = make_shared<SimpleAtomic>();
        model->setParent(this);
        models.push_back(model);
        if (this->simulator != nullptr) {
            this->simulator->pending_schedule.insert(model);
        }
        return false;
    }

  private:
    list<shared_ptr<Devs<SimpleIO>>> models;
};

int main() {
    shared_ptr<SimpleNetwork> model = make_shared<SimpleNetwork>();
    shared_ptr<Simulator<SimpleIO>> sim =
        make_shared<Simulator<SimpleIO>>(model);

    while (sim->nextEventTime() < DBL_MAX && SimpleAtomic::atomic_number < 10) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs + 1 == SimpleAtomic::atomic_number);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
