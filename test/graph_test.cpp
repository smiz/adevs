#include "adevs/graph.h"
#include <cassert>
#include <algorithm>
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
    std::list<std::shared_ptr<Atomic<int,int>>> models;
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
    g.get_atomics(pin0,models);
    g.connect(pin2,a);
    assert(models.size() == 1);
    assert(std::find(models.begin(),models.end(),b) != models.end());
    assert(std::find(models.begin(),models.end(),a) == models.end());
    models.clear();
    g.get_atomics(pin1,models);
    assert(models.size() == 1);
    assert(std::find(models.begin(),models.end(),b) != models.end());
    assert(std::find(models.begin(),models.end(),a) == models.end());
    models.clear();
    g.get_atomics(pin2,models);
    assert(models.size() == 1);
    assert(std::find(models.begin(),models.end(),b) == models.end());
    assert(std::find(models.begin(),models.end(),a) != models.end());
    models.clear();
    g.remove_atomic(a);
    assert(g.get_atomics().find(a) == g.get_atomics().end());
    assert(g.get_atomics().find(b) != g.get_atomics().end());
    g.get_atomics(pin1,models);
    assert(models.size() == 1);
    assert(std::find(models.begin(),models.end(),b) != models.end());
    assert(std::find(models.begin(),models.end(),a) == models.end());
    models.clear();
    g.get_atomics(pin2,models);
    assert(models.size() == 0);
}

int main() {
    test1();
    return 0;
}
