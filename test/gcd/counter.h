#ifndef __counter_h_
#define __counter_h_
#include <cassert>
#include <cstdio>
#include "adevs.h"
#include "object.h"

class counter : public adevs::Atomic<PortValue> {
  public:
    static int const in;

    counter() : adevs::Atomic<PortValue>(), count(0), sigma(DBL_MAX), t(0.0) {}
    void delta_int() { assert(false); }
    void delta_ext(double e, adevs::Bag<PortValue> const &x) {
        t += e;
        count += x.size();
        printf("Count is %d @ %d\n", count, (int)t);
        sigma = DBL_MAX;
    }
    void delta_conf(adevs::Bag<PortValue> const &) { assert(false); }
    void output_func(adevs::Bag<PortValue> &) { assert(false); }
    double ta() { return sigma; }
    void gc_output(adevs::Bag<PortValue> &g) { assert(g.size() == 0); }
    ~counter() {}

  private:
    int count;
    double sigma, t;
};

int const counter::in = 0;

#endif
