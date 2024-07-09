#include <iostream>
#include <list>
#include "SimpleAtomic.h"
#include "adevs.h"
using namespace adevs;
using namespace std;

class SimpleNetwork : public Network<SimpleIO> {
  public:
    SimpleNetwork() : Network<SimpleIO>() {
        SimpleAtomic* model = new SimpleAtomic();
        model->setParent(this);
        models.push_back(model);
    }
    void getComponents(Set<Devs<SimpleIO>*> &c) {
        list<Devs<SimpleIO>*>::iterator iter;
        for (iter = models.begin(); iter != models.end(); iter++) {
            c.insert(*iter);
        }
    }
    void route(SimpleIO const &, Devs<SimpleIO>*, Bag<Event<SimpleIO>> &) {}
    bool model_transition() {
        SimpleAtomic* model = new SimpleAtomic();
        model->setParent(this);
        models.push_back(model);
        return false;
    }
    ~SimpleNetwork() {
        list<Devs<SimpleIO>*>::iterator iter;
        for (iter = models.begin(); iter != models.end(); iter++) {
            delete *iter;
        }
    }

  private:
    list<Devs<SimpleIO>*> models;
};

int main() {
    SimpleNetwork* model = new SimpleNetwork();
    Simulator<SimpleIO>* sim = new Simulator<SimpleIO>(model);
    while (sim->nextEventTime() < DBL_MAX && SimpleAtomic::atomic_number < 10) {
        sim->execNextEvent();
        assert(SimpleAtomic::internal_execs + 1 == SimpleAtomic::atomic_number);
        SimpleAtomic::internal_execs = 0;
    }
    return 0;
}
