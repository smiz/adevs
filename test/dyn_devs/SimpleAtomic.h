#ifndef _simple_atomic_h_
#define _simple_atomic_h_
#include <cassert>
#include "adevs/adevs.h"

typedef adevs::PortValue<char> SimpleIO;

class SimpleAtomic : public adevs::Atomic<SimpleIO> {
  public:
    SimpleAtomic();
    void delta_int();
    void delta_ext(double, adevs::Bag<SimpleIO> const &) { assert(false); }
    void delta_conf(adevs::Bag<SimpleIO> const &) { assert(false); }
    double ta() { return 1.0; }
    void output_func(adevs::Bag<SimpleIO> &) {}

    bool model_transition() { return true; }
    ~SimpleAtomic();

    static int atomic_number;
    static int internal_execs;

  private:
    int number;
};

#endif
