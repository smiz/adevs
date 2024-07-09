#ifndef _Relay_h_
#define _Relay_h_
#include <cassert>
#include "adevs.h"

typedef adevs::PortValue<int> IO_Type;

/**
 * This model accepts a positive input value on port 0 and relays it
 * one time unit later on port 1.
 */
class Relay : public adevs::Atomic<IO_Type> {
  public:
    Relay() : adevs::Atomic<IO_Type>() { relay = -1; }
    void delta_int() { relay = -1; }
    void delta_ext(double e, adevs::Bag<IO_Type> const &r) {
        relay = (*(r.begin())).value;
        assert((*(r.begin())).port == 0);
        assert(relay > 0);
    }
    void delta_conf(adevs::Bag<IO_Type> const &) {}
    double ta() {
        if (relay > 0) {
            return 1.0;
        }
        return DBL_MAX;
    }
    void output_func(adevs::Bag<IO_Type> &y) { y.insert(IO_Type(1, relay)); }
    void gc_output(adevs::Bag<IO_Type> &) {}
    int getRelayValue() const { return relay; }

  private:
    int relay;
};

#endif
