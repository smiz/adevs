#include "adevs/models.h"
#include "adevs/sched.h"

#include <cassert>
#include <iostream>
#include <memory>

using namespace adevs;


class bogus_atomic : public Atomic<char> {
  public:
    bogus_atomic() : Atomic<char>() { _event = (double)rand(); }
    ~bogus_atomic() {}
    void delta_int() {
        // Something happened; pick the next time.
        _event = (double)rand();
    }
    void delta_ext(double, std::list<PinValue<char>> const &xb) {}
    void delta_conf(std::list<PinValue<char>> const &xb) {}
    void output_func(std::list<PinValue<char>> &yb) {}
    double ta() { return (double)_event; }

  private:
    double _event = 0.0;
};


void benchmark_old() {
    Schedule<char> q;

    srand(200);
    list<shared_ptr<bogus_atomic>> keep;
    // Add a bunch of models with random times
    for (int ii = 0; ii < 1000; ii++) {
        shared_ptr<bogus_atomic> m = make_shared<bogus_atomic>();
        keep.push_back(m);
        q.schedule(m, m->ta());
    }

    // Simulate a bunch of things happening
    unsigned long iterations = 100000000;
    cout << "Running: " << iterations << " iterations" << endl;

    unsigned int percentage = 0;
    unsigned long loops_per_percent = iterations / 100;

    // Simulate a bunch of things happening
    for (unsigned long ii = 0; ii < iterations; ii++) {
        if (ii % loops_per_percent == 0) {
            cout << percentage << "%\r" << flush;
            percentage++;
        }
        auto m = q.getMinimum();
        m->delta_int();
        q.schedule(m, m->ta());
    }
    cout << "100%" << "\n" << "Done!" << endl;
}


int main() {

    benchmark_old();

    return 0;
}
