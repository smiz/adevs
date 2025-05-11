#ifndef _simple_atomic_h_
#define _simple_atomic_h_
#include <cassert>
#include "adevs/adevs.h"

class SimpleAtomic : public adevs::Atomic<char> {
  public:
    SimpleAtomic();
    void delta_int();
    void delta_ext(double, std::list<adevs::PinValue<char>> const &) { assert(false); }
    void delta_conf(std::list<adevs::PinValue<char>> const &) { assert(false); }
    double ta() { return 1.0; }
    void output_func(std::list<adevs::PinValue<char>> &) {}
    ~SimpleAtomic();

    static int atomic_number;
    static int internal_execs;

  private:
    int number;
};

#endif
