#include "adevs/schedule2.h"

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


void benchmark_new() {
    Schedule<char> q;

    srand(200);
    // Add a bunch of models with random times
    for (int ii = 0; ii < 1000; ii++) {
        shared_ptr<bogus_atomic> m = make_shared<bogus_atomic>();
        q.add(m);
    }

    // Simulate a bunch of things happening
    cout << "Running" << endl;
    for (unsigned long ii = 0; ii < 100000; ii++) {
        auto m = q.get_next();
        m->delta_int();
        q.update(m);
    }
}


int main() {

    benchmark_new();

    return 0;
}
