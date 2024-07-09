#include <time.h>
#include <iostream>
#include <list>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"
using namespace adevs;
using namespace std;

struct flagged {
    bool flag;
};

class MySimpleAtomic : public SimpleAtomic {
  public:
    MySimpleAtomic() : SimpleAtomic() {}
    bool model_transition() {
        flagged* parent = dynamic_cast<flagged*>(getParent());
        assert(parent->flag == false);
        return true;
    }
    ~MySimpleAtomic() {}
};

class SimpleNetwork : public Network<SimpleIO>, public flagged {
  public:
    SimpleNetwork(int depth = 0)
        : Network<SimpleIO>(), flagged(), depth(depth) {
        flag = false;
        int initial_count = rand() % 3 + 1;
        for (int i = 0; i < initial_count; i++) {
            add_model();
        }
    }
    void getComponents(Set<Devs<SimpleIO>*> &c) {
        list<Devs<SimpleIO>*>::iterator iter;
        for (iter = models.begin(); iter != models.end(); iter++) {
            c.insert(*iter);
        }
    }
    void route(SimpleIO const &, Devs<SimpleIO>*, Bag<Event<SimpleIO>> &) {}
    bool model_transition() {
        flag = true;
        if (getParent() != NULL) {
            flagged* parent = dynamic_cast<flagged*>(getParent());
            assert(parent->flag == false);
        }
        int choice = rand() % 3;
        if (choice == 0) {
            add_model();
            return true;
        } else if (choice == 2 && !models.empty()) {
            models.back()->setParent(NULL);
            models.pop_back();
            return true;
        }
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
    int depth;

    void add_model() {
        Devs<SimpleIO>* model = NULL;
        if (rand() % 2 == 0 || depth == 5) {
            model = new MySimpleAtomic();
        } else {
            model = new SimpleNetwork(depth + 1);
        }
        assert(model != NULL);
        model->setParent(this);
        models.push_front(model);
    }
};

void reset_flags(SimpleNetwork* model) {
    model->flag = false;
    Set<Devs<SimpleIO>*> components;
    model->getComponents(components);
    Set<Devs<SimpleIO>*>::iterator iter;
    for (iter = components.begin(); iter != components.end(); iter++) {
        SimpleNetwork* next_model;
        next_model = dynamic_cast<SimpleNetwork*>(*iter);
        if (next_model != NULL) {
            reset_flags(next_model);
        }
    }
}

void doTest() {
    SimpleNetwork* model = new SimpleNetwork();
    Simulator<SimpleIO>* sim = new Simulator<SimpleIO>(model);
    while (sim->nextEventTime() < 1000.0) {
        reset_flags(model);
        sim->execNextEvent();
    }
    delete sim;
    delete model;
}

int main() {
    unsigned long seed = (unsigned long)time(NULL);
    cout << seed << endl;
    srand(seed);
    for (int i = 0; i < 100; i++) {
        cout << "\r" << i << "\t";
        cout.flush();
        doTest();
    }
    cout << "\rdone\t" << endl;
    return 0;
}
