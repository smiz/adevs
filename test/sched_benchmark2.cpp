#include "adevs/schedule2.h"

#include <cassert>
#include <iostream>
#include <memory>

// using namespace adevs;


class bogus_atomic : public adevs::Atomic<char> {
  public:
    bogus_atomic() : adevs::Atomic<char>() { _event = (double)rand(); }
    ~bogus_atomic() {}
    void delta_int() {
        // Something happened; pick the next time.
        _event = (double)rand();
    }
    void delta_ext(double, std::list<adevs::PinValue<char>> const &xb) {}
    void delta_conf(std::list<adevs::PinValue<char>> const &xb) {}
    void output_func(std::list<adevs::PinValue<char>> &yb) {}
    double ta() { return (double)_event; }

  private:
    double _event = 0.0;
};


void benchmark_new() {
	adevs::Schedule<char> q;

    srand(200);
    // Add a bunch of models with random times
    for (int ii = 0; ii < 1000; ii++) {
        std::shared_ptr<bogus_atomic> m = std::make_shared<bogus_atomic>();
        q.add(m);
    }

    // Simulate a bunch of things happening
    unsigned long iterations = 100000000;
    std::cout << "Running: " << iterations << " iterations" << std::endl;

    unsigned int percentage = 0;
    unsigned long loops_per_percent = iterations / 100;

    for (unsigned long ii = 0; ii < iterations; ii++) {
        if (ii % loops_per_percent == 0) {
            std::cout << percentage << "%\r" << std::flush;
            percentage++;
        }
        auto m = q.get_next();
        m->delta_int();
        q.update(m);
    }
    std::cout << "100%" << "\n" << "Done!" << std::endl;
}


int main() {

    benchmark_new();

    return 0;
}
