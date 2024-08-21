#ifndef __Generator_h_
#define __Generator_h_

#include <memory>
#include <random>

#include "adevs/adevs.h"

/*
 * The Generator models factory demand. It creates new orders every 0.5 to 2 days.
 */
class Generator : public adevs::Atomic<int> {
  public:
    /*
     * The generator requires a seed for the random number that determines
     * the time between new orders.
     */
    Generator(unsigned long seed, float min = 0.5, float max = 2.0)
        : adevs::Atomic<int>(),
          next(1),
          generator(std::make_unique<std::mt19937>(seed)),
          distribution(
              std::make_unique<std::uniform_real_distribution<>>(min, max)) {
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
    double ta() { return (double)time_to_order; }

    // Model is input free, so these methods are empty
    void delta_ext(double, adevs::Bag<int> const &) {}
    void delta_conf(adevs::Bag<int> const &) {}

  private:
    // Next order ID
    int next;

    // Time until that order arrives
    float time_to_order;

    // Random variable for producing order arrival times
    std::unique_ptr<std::mt19937> generator;
    std::unique_ptr<std::uniform_real_distribution<>> distribution;

    // Method to set the order time
    void set_time_to_order() { time_to_order = (*distribution)(*generator); }
};

#endif
