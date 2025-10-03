#ifndef _relay_h_
#define _relay_h_

#include <cassert>
#include <memory>
#include <vector>
#include "adevs/adevs.h"

using pin_t = adevs::pin_t;
using Atomic = adevs::Atomic<int>;
using PinValue = adevs::PinValue<int>;

/**
 * This model accepts a positive input value on pin in and relays it
 * one time unit later on pin out.
 */
class Relay : public Atomic {
  public:
    Relay() : Atomic() { relay = -1; }

    pin_t const in, out;

    void delta_int() { relay = -1; }

    void delta_ext(double, std::list<PinValue> const &r) {
        relay = (*(r.begin())).value;
        assert((*(r.begin())).pin == in);
        assert(relay > 0);
    }

    void delta_conf(std::list<PinValue> const &) {}

    double ta() {
        if (relay > 0) {
            return 1.0;
        }
        return adevs_inf<double>();
    }

    void output_func(std::list<PinValue> &y) { y.push_back(PinValue(out, relay)); }


    int getRelayValue() const { return relay; }

  private:
    int relay;
};

#endif
