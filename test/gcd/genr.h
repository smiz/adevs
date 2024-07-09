#ifndef __genr_h_
#define __genr_h_
#include <cassert>
#include <cstdio>
#include <vector>
#include "adevs/adevs.h"
#include "object.h"

class genr : public adevs::Atomic<PortValue> {
  public:
    static int const stop;
    static int const start;
    static int const signal;

    genr(std::vector<double> const &pattern, int iterations, bool active = true)
        : adevs::Atomic<PortValue>(),
          pattern(pattern),
          active(active),
          init_state(active),
          iterations(iterations) {
        init();
    }
    genr(double period, int iterations, bool active = true)
        : adevs::Atomic<PortValue>(),
          active(active),
          init_state(active),
          iterations(iterations) {
        pattern.push_back(period);
        init();
    }
    void init() {
        active = init_state;
        count = 1;
        if (!active) {
            sigma = DBL_MAX;
        } else {
            assert(pattern[0] >= 0.0);
            sigma = pattern[0];
        }
    }
    void delta_int() {
        if (count >= iterations) {
            sigma = DBL_MAX;
        } else {
            assert(pattern[count % pattern.size()] >= 0.0);
            sigma = pattern[count++ % pattern.size()];
        }
    }
    void delta_ext(double, adevs::Bag<PortValue> const &x) {
        sigma = DBL_MAX;
        active = false;
        adevs::Bag<PortValue>::const_iterator i;
        for (i = x.begin(); i != x.end(); i++) {
            if ((*i).port == start) {
                if (active == false) {
                    active = true;
                    count = 1;
                    assert(pattern[0] >= 0.0);
                    sigma = pattern[0];
                    return;
                }
            }
        }
        printf("Got genr.stop\n");
    }
    void delta_conf(adevs::Bag<PortValue> const &x) { delta_ext(ta(), x); }
    void output_func(adevs::Bag<PortValue> &y) {
        PortValue pv;
        pv.port = signal;
        pv.value = new object();
        y.insert(pv);
    }
    double ta() { return sigma; }
    void gc_output(adevs::Bag<PortValue> &g) {
        adevs::Bag<PortValue>::iterator i;
        for (i = g.begin(); i != g.end(); i++) {
            delete (*i).value;
        }
    }
    ~genr() {}

  private:
    std::vector<double> pattern;
    bool active;
    bool init_state;
    int iterations;
    int count;
    double sigma;
};

int const genr::stop = 0;
int const genr::start = 1;
int const genr::signal = 2;

#endif
