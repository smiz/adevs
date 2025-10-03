#ifndef _simple_atomic_h_
#define _simple_atomic_h_
#include <cassert>
#include "adevs/adevs.h"

using Atomic = adevs::Atomic<char>;
using PinValue = adevs::PinValue<char>;

class SimpleAtomic : public Atomic {
  public:
    SimpleAtomic();
    void delta_int();
    void delta_ext(double, std::list<PinValue> const &) { assert(false); }
    void delta_conf(std::list<PinValue> const &) { assert(false); }
    double ta() { return 1.0; }
    void output_func(std::list<PinValue> &) {}
    ~SimpleAtomic();

    static int atomic_number;
    static int internal_execs;

  private:
    int number;
};

#endif
