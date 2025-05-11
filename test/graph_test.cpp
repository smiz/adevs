#include "adevs/graph.h"
#include <cassert>
#include <algorithm>
#include <map>
using namespace adevs;

class TestAtomic:
    public Atomic<int,int> {
    public:
        TestAtomic():
            Atomic<int,int>() {
        }
        void delta_int() {
        }
        void delta_ext(int, const std::list<PinValue<int>>&) {
        }
        void delta_conf(const std::list<PinValue<int>>&) {
        }
        void output_func(std::list<PinValue<int>>&) {
        }
        int ta() { return 0; }
    };

void test1() {
    std::list<std::pair<pin_t,std::shared_ptr<Atomic<int,int>>>> models;
    std::shared_ptr<Atomic<int,int>> a(new TestAtomic());
    std::shared_ptr<Atomic<int,int>> b(new TestAtomic());
    Graph<int,int> g;
    g.add_atomic(a);
    g.add_atomic(b);
    assert(g.get_atomics().find(a) != g.get_atomics().end());
    assert(g.get_atomics().find(b) != g.get_atomics().end());
    pin_t pin0 = g.add_pin();
    pin_t pin1 = g.add_pin();
    pin_t pin2 = g.add_pin();
    assert(pin0 != pin1);
    g.connect(pin0,pin1);
    g.connect(pin1,b);
    g.connect(pin2,a);
    g.route(pin0,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == b);
    models.clear();
    g.route(pin1,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == b);
    models.clear();
    g.route(pin2,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == a);
    models.clear();
    g.remove_atomic(a);
    assert(g.get_atomics().find(a) == g.get_atomics().end());
    assert(g.get_atomics().find(b) != g.get_atomics().end());
    g.route(pin1,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == b);
    models.clear();
    g.route(pin2,models);
    assert(models.size() == 0);
}

void test2() {
    std::list<std::pair<pin_t,std::shared_ptr<Atomic<int,int>>>> models;
    std::shared_ptr<Atomic<int,int>> a(new TestAtomic());
    Graph<int,int> g;
    pin_t pin0 = g.add_pin();
    pin_t pin1 = g.add_pin();
    g.set_provisional(true);
    g.add_atomic(a);
    assert(g.get_atomics().find(a) == g.get_atomics().end());
    assert(g.get_pending().back().model == a);
    assert(g.get_pending().back().op == g.ADD_ATOMIC);
    g.remove_atomic(a);
    assert(g.get_atomics().find(a) == g.get_atomics().end());
    assert(g.get_pending().back().model == a);
    assert(g.get_pending().back().op == g.REMOVE_ATOMIC);
    g.connect(pin0,pin1);
    assert(g.get_pending().back().pin[0] == pin0);
    assert(g.get_pending().back().pin[1] == pin1);
    assert(g.get_pending().back().op == g.CONNECT_PIN_TO_PIN);
    g.route(pin0,models);
    assert(models.empty());
    g.route(pin1,models);
    assert(models.empty());
    g.disconnect(pin0,pin1);
    assert(g.get_pending().back().pin[0] == pin0);
    assert(g.get_pending().back().pin[1] == pin1);
    assert(g.get_pending().back().op == g.DISCONNECT_PIN_FROM_PIN);
    g.connect(pin0,a);
    assert(g.get_pending().back().pin[0] == pin0);
    assert(g.get_pending().back().model == a);
    assert(g.get_pending().back().op == g.CONNECT_PIN_TO_ATOMIC);
    g.route(pin0,models);
    assert(models.empty());
    g.disconnect(pin0,a);
    assert(g.get_pending().back().pin[0] == pin0);
    assert(g.get_pending().back().model == a);
    assert(g.get_pending().back().op == g.DISCONNECT_PIN_FROM_ATOMIC);
    g.remove_pin(pin0);
    assert(g.get_pending().back().pin[0] == pin0);
    assert(g.get_pending().back().op == g.REMOVE_PIN);
}

int main() {
    test1();
    test2();
    return 0;
}
