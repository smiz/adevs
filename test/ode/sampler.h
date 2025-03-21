#ifndef _sampler_h_
#define _sampler_h_
#include <iostream>
#include "adevs/adevs.h"

/**
Sampling function for continuous system simulator testing.
All input and output is through the sample pin.
*/
class sampler : public adevs::Atomic<double> {
  public:
    sampler(double dt)
        : adevs::Atomic<double>(),
          dt(dt),
          sigma(dt),
          t(0.0) {}

    void delta_int() {
        t += sigma;
        sigma = dt;
    }

    void delta_ext(double e, list<adevs::PinValue<double>> const &xb) {
        sigma -= e;
        t += e;
        std::cout << t << " ";
        list<adevs::PinValue<double>>::const_iterator iter;
        for (iter = xb.begin(); iter != xb.end(); iter++) {
            std::cout << (*iter).value << " ";
        }
        std::cout << std::endl;
    }

    void delta_conf(list<adevs::PinValue<double>> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }

    double ta() { return sigma; }

    void output_func(list<adevs::PinValue<double>> &yb) {
        adevs::PinValue<double> event(sample_pin, 0.0);
        yb.push_back(event);
    }

    int sample_pin;

  private:
    double const dt;
    double sigma, t;
};

#endif
