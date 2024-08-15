#include "SimpleModel.h"
#include "Wrapper.h"
using namespace std;
using namespace adevs;

int External::num_existing = 0;
int Internal::num_existing = 0;

static Wrapper* model;
static bool output_happened = false;

class Listener : public EventListener<External*> {
  public:
    Listener() : EventListener<External*>() {}
    void setExpected(double t, Speed s, int v) {
        this->t = t;
        this->s = s;
        this->v = v;
    }
    void outputEvent(Event<External*> e, double t) {
        assert(e.model == model);
        assert(e.value->speed == s);
        assert(e.value->value == v);
        assert(t == this->t);
        output_happened = true;
    }

  private:
    double t;
    Speed s;
    int v;
};

int main() {
    model = new Wrapper(new SimpleModel());
    Simulator<External*>* sim = new Simulator<External*>(model);
    Listener* l = new Listener();
    sim->addEventListener(l);
    // First input/output series. Internal event test.
    l->setExpected(1.0, FAST, 1);
    Event<External*> e;
    e.model = model;
    e.value = new External(FAST, 1);
    Bag<Event<External*>> x;
    x.push_back(e);
    sim->computeNextState(x, 0.0);
    assert(sim->nextEventTime() == 1.0);
    sim->execNextEvent();
    assert(output_happened);
    output_happened = false;
    // Second input/output series. External event test.
    l->setExpected(3.5, SLOW, 1);
    x.clear();
    e.value = new External(SLOW, 1);
    x.push_back(e);
    sim->computeNextState(x, 1.5);
    assert(!output_happened);
    assert(sim->nextEventTime() == 3.5);
    sim->execNextEvent();
    assert(output_happened);
    output_happened = false;
    // Third input/output series. Confluent event test
    l->setExpected(5.5, SLOW, 2);
    x.clear();
    e.value = new External(STOP, 1);
    x.push_back(e);
    assert(sim->nextEventTime() == 5.5);
    sim->computeNextState(x, sim->nextEventTime());
    assert(output_happened);
    assert(sim->nextEventTime() == DBL_MAX);
    // Done. Try to clean up.
    assert(External::num_existing == 3);
    delete model;
    delete l;
    delete sim;
    assert(Internal::num_existing == 0);
    return 0;
}
