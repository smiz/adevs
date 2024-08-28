#include <time.h>
#include <iostream>
#include <list>
#include <memory>
#include "SimpleAtomic.h"
#include "adevs/adevs.h"

using namespace adevs;
using namespace std;


struct flagged {
    bool flag;
};

enum class TYPE {
    NETWORK,
    ATOMIC,
};

class MySimpleAtomic : public SimpleAtomic {
  public:
    MySimpleAtomic() : SimpleAtomic() { printf("atomic():  this=%p\n", this); }

    bool model_transition() {
        flagged* parent = dynamic_cast<flagged*>(getParent());
        printf("atomic::transition:  this=%p parent=%p flag=%d\n", this, parent,
               parent->flag);
        assert(parent->flag == false);
        return true;
    }
};

class SimpleNetwork : public Network<SimpleIO>, public flagged {
  public:
    SimpleNetwork(int depth = 0)
        : Network<SimpleIO>(), flagged(), depth(depth) {
        flag = false;
        int initial_count = rand() % 3 + 1;
        printf("network(): this=%p initial_count=%d depth=%d\n", this,
               initial_count, depth);
        for (int i = 0; i < initial_count; i++) {
            add_model(depth, i);
        }
    }
    void getComponents(set<Devs<SimpleIO>*> &c) {
        for (auto iter : models) {
            c.insert(iter.get());
        }
    }

    void route(SimpleIO const &, Devs<SimpleIO>*, list<Event<SimpleIO>> &) {}

    bool model_transition() {
        flag = true;
        if (getParent() != nullptr) {
            flagged* parent = dynamic_cast<flagged*>(getParent());
            printf("network::transition: (check) this=%p parent=%p flag=%d\n",
                   this, parent, parent->flag);
            assert(parent->flag == false);
        }
        int choice = rand() % 3;
        if (choice == 0) {
            printf("network::transition: (add) this=%p\n", this);
            add_model(depth, -1);
            return true;
        } else if (choice == 2 && !models.empty()) {
            models.back()->setParent(nullptr);
            printf("network::transition: (remove) this=%p model=%p\n", this,
                   models.back().get());
            if (this->simulator != nullptr) {
                this->simulator->pending_unschedule.insert(models.back());
            }
            models.pop_back();
            return true;
        }
        return false;
    }

  private:
    list<shared_ptr<Devs<SimpleIO>>> models;
    int depth;

    void add_model(int depth, int ii) {
        shared_ptr<Devs<SimpleIO>> model = nullptr;
        TYPE t;
        if (rand() % 2 == 0 || depth == 5) {
            model = make_shared<MySimpleAtomic>();
            t = TYPE::ATOMIC;
        } else {
            model = make_shared<SimpleNetwork>(depth + 1);
            t = TYPE::NETWORK;
        }
        assert(model != nullptr);
        printf("created[%d, %d]: type=%d model=%p parent=%p\n", depth, ii, t,
               model.get(), this);
        model->setParent(this);
        models.push_front(model);
        if (this->simulator != nullptr) {
            this->simulator->pending_schedule.insert(model);
        }
    }
};

void reset_flags(SimpleNetwork* model) {
    model->flag = false;
    set<Devs<SimpleIO>*> components;
    model->getComponents(components);

    for (auto iter : components) {
        SimpleNetwork* next_model;
        next_model = dynamic_cast<SimpleNetwork*>(iter);
        if (next_model != nullptr) {
            reset_flags(next_model);
        }
    }
}

void doTest() {
    shared_ptr<SimpleNetwork> model = make_shared<SimpleNetwork>();
    shared_ptr<Simulator<SimpleIO>> sim =
        make_shared<Simulator<SimpleIO>>(model);

    while (sim->nextEventTime() < 1000.0) {
        reset_flags(model.get());
        sim->execNextEvent();
    }
}

int main() {
    unsigned long seed = (unsigned long)time(NULL);

    cout << seed << endl;
    srand(seed);

    for (int i = 0; i < 100; i++) {
        cout << "\r" << i << endl;  //"\t";
        cout.flush();
        doTest();
    }

    cout << "\rdone\t" << endl;
    return 0;
}
