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
        for (int i = 0; i < 10; i++) {
            shared_ptr<SimpleAtomic> model = make_shared<SimpleAtomic>();
            model->setParent(this);
            models.push_back(model);
        }
    }
    void getComponents(set<Devs<SimpleIO>*> &c) {
        for (auto iter : models) {
            c.insert(iter.get());
        }
    }
    void route(SimpleIO const &, Devs<SimpleIO>*, list<Event<SimpleIO>> &) {}

    bool model_transition() {
        if (this->simulator != nullptr) {
            this->simulator->pending_unschedule.insert(models.back());
        }
        models.pop_back();
        return false;
    }

  private:
    list<shared_ptr<Devs<SimpleIO>>> models;
};

int main() {
    shared_ptr<SimpleNetwork> model = make_shared<SimpleNetwork>();
    shared_ptr<Simulator<SimpleIO>> sim =
        make_shared<Simulator<SimpleIO>>(model);

    while (sim->nextEventTime() < DBL_MAX && SimpleAtomic::atomic_number != 0) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs == SimpleAtomic::atomic_number + 1);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
