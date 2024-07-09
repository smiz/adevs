#ifndef _SimpleModel_h_
#define _SimpleModel_h_
#include "events.h"

/**
 * This is a simple model that generates periodic outputs. The rate is
 * and initial counter value are set with an input, and the rate and
 * current counter are produced as output.
 */
class SimpleModel : public adevs::Atomic<Internal*> {
  public:
    SimpleModel() : adevs::Atomic<Internal*>() {}
    void delta_int() { s.value++; }
    void delta_ext(double e, adevs::Bag<Internal*> const &xb) {
        adevs::Bag<Internal*>::const_iterator iter = xb.begin();
        s.value = (*iter)->value;
        s.period = (*iter)->period;
    }
    void delta_conf(adevs::Bag<Internal*> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    double ta() { return s.period; }
    void output_func(adevs::Bag<Internal*> &yb) { yb.insert(new Internal(s)); }
    void gc_output(adevs::Bag<Internal*> &g) {
        adevs::Bag<Internal*>::const_iterator iter = g.begin();
        for (; iter != g.end(); iter++) {
            delete *iter;
        }
    }

  private:
    Internal s;
};

#endif
