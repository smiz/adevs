#ifndef _genr_h_
#define _genr_h_
#include <cassert>
#include <cstdio>
#include <vector>
#include "adevs/adevs.h"
#include "object.h"

class genr : public adevs::Atomic<ObjectPtr> {
  public:
    adevs::pin_t stop;
    adevs::pin_t start;
    adevs::pin_t signal;

    genr(std::vector<double> const &pattern, int iterations, bool active = true)
        : adevs::Atomic<ObjectPtr>(),
          pattern(pattern),
          active(active),
          init_state(active),
          iterations(iterations) {
        init();
    }
    genr(double period, int iterations, bool active = true)
        : adevs::Atomic<ObjectPtr>(),
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
    void delta_ext(double, std::list<adevs::PinValue<ObjectPtr>> const &x) {
        sigma = adevs_inf<double>();
        active = false;
        std::list<adevs::PinValue<ObjectPtr>>::const_iterator i;
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
    void delta_conf(std::list<adevs::PinValue<ObjectPtr>> const &x) { delta_ext(ta(), x); }
    void output_func(std::list<adevs::PinValue<ObjectPtr>> &y) {
        adevs::PinValue<ObjectPtr> pv;
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
