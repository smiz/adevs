#include "adevs/graph.h"
#include <cassert>
#include <algorithm>
#include <map>
// using namespace adevs;

class TestAtomic:
    public adevs::Atomic<int,int> {
    public:
        TestAtomic():
            Atomic<int,int>() {
        }
        void delta_int() {
        }
        void delta_ext(int, const std::list<adevs::PinValue<int>>&) {
        }
        void delta_conf(const std::list<adevs::PinValue<int>>&) {
        }
        void output_func(std::list<adevs::PinValue<int>>&) {
        }
        int ta() { return 0; }
    };

void test1() {
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    std::shared_ptr<adevs::Atomic<int,int>> b(new TestAtomic());
    adevs::Graph<int,int> g;
    g.add_atomic(a);
    g.add_atomic(b);
    assert(g.get_atomics().find(a) != g.get_atomics().end());
    assert(g.get_atomics().find(b) != g.get_atomics().end());
    adevs::pin_t pin0, pin1, pin2;
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
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    adevs::Graph<int,int> g;
    adevs::pin_t pin0, pin1;
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

void test3() {
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    adevs::Graph<int,int> g;
    adevs::pin_t pin0, pin1, pin2;
    g.add_atomic(a);
    g.connect(pin0,pin1);
    g.connect(pin1,pin2);
    g.connect(pin2,a);
    g.route(pin0,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == a);
    assert((*(models.begin())).first == pin2);
}

void test4() {
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    adevs::Graph<int,int> g;
    g.add_atomic(a);
    g.add_atomic(a);
    g.remove_atomic(a);
    assert(g.get_atomics().find(a) != g.get_atomics().end());
    g.remove_atomic(a);
    assert(g.get_atomics().find(a) == g.get_atomics().end());
}

void test5() {
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    adevs::Graph<int,int> g;
    adevs::pin_t pin0, pin1;
    g.connect(pin0,pin1);
    g.connect(pin0,pin1);
    g.connect(pin1,a);
    g.connect(pin1,a);
    g.route(pin0,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == a);
    assert((*(models.begin())).first == pin1);
    models.clear();
    g.disconnect(pin0,pin1);
    g.route(pin0,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == a);
    assert((*(models.begin())).first == pin1);
    models.clear();
    g.disconnect(pin0,pin1);
    g.route(pin0,models);
    assert(models.empty());
    g.route(pin1,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == a);
    assert((*(models.begin())).first == pin1);
    models.clear();
    g.disconnect(pin1,a);
    g.route(pin1,models);
    assert(models.size() == 1);
    assert((*(models.begin())).second == a);
    assert((*(models.begin())).first == pin1);
    models.clear();
    g.disconnect(pin1,a);
    g.route(pin1,models);
    assert(models.empty());
}

void test6() {
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    std::shared_ptr<adevs::Atomic<int,int>> b(new TestAtomic());
    adevs::Graph<int,int> g;
    adevs::pin_t pin0, pin1, pin2;
    g.add_atomic(a);
    g.add_atomic(b);
    g.add_atomic(b);
    g.connect(pin0,pin1);
    g.connect(pin0,pin2);
    g.connect(pin1,a);
    g.connect(pin2,b);
    g.route(pin0,models);
    assert(models.size() == 2);
    assert(std::find(models.begin(),models.end(),std::make_pair(pin1,a)) != models.end());
    assert(std::find(models.begin(),models.end(),std::make_pair(pin2,b)) != models.end());
    models.clear();
    g.remove_atomic(b);
    assert(g.get_atomics().find(b) != g.get_atomics().end());
    g.route(pin0,models);
    assert(models.size() == 2);
    assert(std::find(models.begin(),models.end(),std::make_pair(pin1,a)) != models.end());
    assert(std::find(models.begin(),models.end(),std::make_pair(pin2,b)) != models.end());
    models.clear();
    g.remove_atomic(b);
    assert(g.get_atomics().find(b) == g.get_atomics().end());
    g.route(pin0,models);
    assert(models.size() == 1);
    assert(std::find(models.begin(),models.end(),std::make_pair(pin1,a)) != models.end());
}

void test7() {
    std::list<std::pair<adevs::pin_t,std::shared_ptr<adevs::Atomic<int,int>>>> models;
    std::shared_ptr<adevs::Atomic<int,int>> a(new TestAtomic());
    std::shared_ptr<adevs::Atomic<int,int>> b(new TestAtomic());
    std::shared_ptr<adevs::Atomic<int,int>> c(new TestAtomic());
    adevs::Graph<int,int> g;
    adevs::pin_t pin0, pin1, pin2;
    g.add_atomic(a);
    g.add_atomic(b);
    g.connect(pin0,pin1);
    g.connect(pin0,pin2);
    g.connect(pin0,pin2);
    g.connect(pin1,a);
    g.connect(pin2,b);
    g.connect(pin2,b);
    g.connect(pin2,c);
    g.route(pin0,models);
    assert(models.size() == 3);
    assert(std::find(models.begin(),models.end(),std::make_pair(pin1,a)) != models.end());
    assert(std::find(models.begin(),models.end(),std::make_pair(pin2,b)) != models.end());
    assert(std::find(models.begin(),models.end(),std::make_pair(pin2,c)) != models.end());
    models.clear();
    g.remove_pin(pin2);
    g.route(pin0,models);
    assert(models.size() == 2);
    assert(std::find(models.begin(),models.end(),std::make_pair(pin1,a)) != models.end());
    assert(std::find(models.begin(),models.end(),std::make_pair(pin2,b)) != models.end());
    models.clear();
    g.remove_pin(pin2);
    g.route(pin0,models);
    assert(models.size() == 1);
    assert(std::find(models.begin(),models.end(),std::make_pair(pin1,a)) != models.end());
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    return 0;
}
