#ifndef _genr_h_
#define _genr_h_
#include <cassert>
#include <cstdio>
#include <vector>
#include "adevs/adevs.h"
#include "object.h"

using pin_t = adevs::pin_t;
using Atomic = adevs::Atomic<ObjectPtr>;

class genr : public Atomic {
  public:
    pin_t stop;
    pin_t start;
    pin_t signal;

    genr(std::vector<double> const &pattern, int iterations, bool active = true)
        : Atomic(), pattern(pattern), active(active), init_state(active), iterations(iterations) {
        init();
    }
    genr(double period, int iterations, bool active = true)
        : Atomic(), active(active), init_state(active), iterations(iterations) {
        pattern.push_back(period);
        init();
    }
    void init() {
        active = init_state;
        count = 1;
        if (!active) {
            sigma = adevs_inf<double>();
        } else {
            assert(pattern[0] >= 0.0);
            sigma = pattern[0];
        }
    }
    void delta_int() {
        if (count >= iterations) {
            sigma = adevs_inf<double>();
        } else {
            assert(pattern[count % pattern.size()] >= 0.0);
            sigma = pattern[count++ % pattern.size()];
        }
    }
    void delta_ext(double, std::list<PinValue> const &x) {
        sigma = adevs_inf<double>();
        active = false;
        std::list<PinValue>::const_iterator i;
        for (i = x.begin(); i != x.end(); i++) {
            if ((*i).pin == start) {
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
    void delta_conf(std::list<PinValue> const &x) { delta_ext(ta(), x); }
    void output_func(std::list<PinValue> &y) {
        PinValue pv;
        pv.pin = signal;
        pv.value = std::make_shared<object>();
        y.push_back(pv);
    }
    double ta() { return sigma; }

  private:
    std::vector<double> pattern;
    bool active;
    bool init_state;
    int iterations;
    int count;
    double sigma;
};

#endif
