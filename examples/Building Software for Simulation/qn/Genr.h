#ifndef _Sw_h_
#define _Sw_h_
#include <cstdlib>
#include <iostream>
#include "adevs/adevs.h"

class Genr : public adevs::Atomic<int> {
  public:
    Genr(int freq)
        : adevs::Atomic<int>(), events(0), period(1.0 / (double)freq) {
        srandom(1);
    }
    void delta_int() { events++; }
    void delta_ext(double, adevs::Bag<int> const &) {}
    void delta_conf(adevs::Bag<int> const &) {}
    double ta() { return period; }
    void output_func(adevs::Bag<int> &yb) { yb.insert(random()); }
    void gc_output(adevs::Bag<int> &) {}
    double lookahead() { return DBL_MAX; }

  private:
    unsigned int events;
    double const period;
};

class Collector : public adevs::Atomic<int> {
  public:
    Collector() : adevs::Atomic<int>(), events(0) {}
    void delta_int() {}
    void delta_ext(double, adevs::Bag<int> const &xb) { events += xb.size(); }
    void delta_conf(adevs::Bag<int> const &) {}
    double ta() { return DBL_MAX; }
    void output_func(adevs::Bag<int> &) {}
    void gc_output(adevs::Bag<int> &) {}
    double lookahead() { return DBL_MAX; }

  private:
    unsigned int events;
};

#endif
