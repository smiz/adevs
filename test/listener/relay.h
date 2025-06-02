#ifndef _relay_h_
#define _relay_h_

#include <cassert>
#include <memory>
#include <vector>
#include "adevs/adevs.h"

/**
 * This model accepts a positive input value on pin in and relays it
 * one time unit later on pin out.
 */
class Relay : public adevs::Atomic<int> {
  public:
    Relay() : adevs::Atomic<int>() { relay = -1; }

    const adevs::pin_t in, out;

    void delta_int() { relay = -1; }

    void delta_ext(double, std::list<adevs::PinValue<int>> const &r) {
        relay = (*(r.begin())).value;
        assert((*(r.begin())).pin == in);
        assert(relay > 0);
    }

    void delta_conf(std::list<adevs::PinValue<int>> const &) {}

    double ta() {
        if (relay > 0) {
            return 1.0;
        }
        return adevs_inf<double>();
    }

    void output_func(std::list<adevs::PinValue<int>> &y) { y.push_back(adevs::PinValue<int>(out, relay)); }


    int getRelayValue() const { return relay; }

  private:
    int relay;
};

#endif
