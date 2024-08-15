#ifndef __Genr_h_
#define __Genr_h_
#include <random>
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
    Genr(unsigned long seed) : adevs::Atomic<int>(), next(1) {
        set_time_to_order();
    }

    // Internal transition updates the order counter and determines the next arrival time
    void delta_int() {
        next++;
        set_time_to_order();
    }

    // Output function produces the next order
    void output_func(adevs::Bag<int> &yb) { yb.push_back(next); }

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
    std::random_device rd = std::random_device();
    std::mt19937 generator = std::mt19937(rd());
    std::uniform_real_distribution<double> distribution =
        std::uniform_real_distribution<double>(0.5, 2.0);

    // Method to set the order time
    void set_time_to_order() { time_to_order = distribution(generator); }
};

#endif
