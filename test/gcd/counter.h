#ifndef _counter_h_
#define _counter_h_
#include <cassert>
#include <cstdio>
#include "adevs/adevs.h"
#include "object.h"

using pin_t = adevs::pin_t;
using Atomic = adevs::Atomic<ObjectPtr>;
using PinValue = adevs::PinValue<ObjectPtr>;

class counter : public Atomic {
  public:
    pin_t in;

    counter() : Atomic(), count(0), sigma(adevs_inf<double>()), t(0.0) {}
    void delta_int() { assert(false); }
    void delta_ext(double e, std::list<PinValue> const &x) {
        t += e;
        count += x.size();
        printf("Count is %d @ %d\n", count, (int)t);
        sigma = adevs_inf<double>();
    }
    void delta_conf(std::list<PinValue> const &) { assert(false); }
    void output_func(std::list<PinValue> &) { assert(false); }
    double ta() { return sigma; }

  private:
    int count;
    double sigma, t;
};

#endif
