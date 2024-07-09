#ifndef __Genr_h_
#define __Genr_h_
#include "adevs/adevs.h"

/**
 * The Genr models factory demand. It creates new orders every 0.5 to 2 days.
 */
class Genr : public adevs::Atomic<int> {
  public:
    /**
		 * The generator requires a seed for the random number that determines
		 * the time between new orders.
		 */
    Genr(unsigned long seed) : adevs::Atomic<int>(), next(1), u(seed) {
        set_time_to_order();
    }
    // Internal transition updates the order counter and determines the next arrival time
    void delta_int() {
        next++;
        set_time_to_order();
    }
    // Output function produces the next order
    void output_func(adevs::Bag<int> &yb) { yb.insert(next); }
    // Time advance returns the time until the next order
    double ta() { return time_to_order; }
    // Model is input free, so these methods are empty
    void delta_ext(double, adevs::Bag<int> const &) {}
    void delta_conf(adevs::Bag<int> const &) {}
    // No explicit memory management is needed
    void gc_output(adevs::Bag<int> &) {}

  private:
    // Next order ID
    int next;
    // Time until that order arrives
    double time_to_order;
    // Random variable for producing order arrival times
    adevs::rv u;
    // Method to set the order time
    void set_time_to_order() { time_to_order = u.uniform(0.5, 2.0); }
};

#endif
