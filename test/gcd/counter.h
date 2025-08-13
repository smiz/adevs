#ifndef _counter_h_
#define _counter_h_
#include <cassert>
#include <cstdio>
#include "adevs/adevs.h"
#include "object.h"

class counter : public adevs::Atomic<ObjectPtr> {
  public:
    adevs::pin_t in;

    counter() : adevs::Atomic<ObjectPtr>(),
      count(0), sigma(adevs_inf<double>()), t(0.0) {}
    void delta_int() { assert(false); }
    void delta_ext(double e, std::list<adevs::PinValue<ObjectPtr>> const &x) {
        t += e;
        count += x.size();
        printf("Count is %d @ %d\n", count, (int)t);
        sigma = adevs_inf<double>();
    }
    void delta_conf(std::list<adevs::PinValue<ObjectPtr>> const &) { assert(false); }
    void output_func(std::list<adevs::PinValue<ObjectPtr>> &) { assert(false); }
    double ta() { return sigma; }

  private:
    int count;
    double sigma, t;
};

#endif
