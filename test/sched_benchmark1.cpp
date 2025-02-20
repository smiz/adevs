#include "adevs/sched.h"

#include <cassert>
#include <iostream>
#include <memory>

using namespace adevs;


class bogus_atomic : public Atomic<char> {
  public:
    bogus_atomic() : Atomic<char>() { _event = (double)rand(); }
    void delta_int() {
        // Something happened; pick the next time.
        _event = (double)rand();
    }
    void delta_ext(double, list<char> const &) {}
    void delta_conf(list<char> const &) {}
    void output_func(list<char> &) {}

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
        q.schedule(m.get(), m->ta());
    }

    // Simulate a bunch of things happening
    for (unsigned long ii = 0; ii < 100000; ii++) {
        auto m = q.getMinimum();
        m->delta_int();
        q.schedule(m, m->ta());
    }
}


int main() {

    benchmark_old();

    return 0;
}
