#ifndef _sampler_h_
#define _sampler_h_
#include <iostream>
#include "adevs/adevs.h"

using Atomic = adevs::Atomic<double>;
using PinValue = adevs::PinValue<double>;
using pin_t = adevs::pin_t;

/**
Sampling function for continuous system simulator testing.
All input and output is through the sample pin.
*/
class sampler : public Atomic {
  public:
    sampler(double dt) : Atomic(), dt(dt), sigma(dt), t(0.0) {}

    void delta_int() {
        t += sigma;
        sigma = dt;
    }

    void delta_ext(double e, std::list<PinValue> const &xb) {
        sigma -= e;
        t += e;
        std::cout << t << " ";
        std::list<PinValue>::const_iterator iter;
        for (iter = xb.begin(); iter != xb.end(); iter++) {
            std::cout << (*iter).value << " ";
        }
        std::cout << std::endl;
    }

    void delta_conf(std::list<PinValue> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }

    double ta() { return sigma; }

    void output_func(std::list<PinValue> &yb) {
        PinValue event(sample_pin, 0.0);
        yb.push_back(event);
    }

    pin_t const sample_pin;

  private:
    double const dt;
    double sigma, t;
};

#endif
