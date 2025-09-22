#ifndef _Machine_h_
#define _Machine_h_
#include "adevs/adevs.h"

using Atomic = adevs::Atomic<int>;

class Machine : public Atomic {
  public:
    Machine(double tm) : Atomic(), tm(tm), p(0), sigma(tm) {}
    void delta_int() {
        p--;
        sigma = tm;
    }
    void delta_ext(double e, std::list<int> const &xb) {
        if (p > 0) {
            sigma -= e;
        } else {
            sigma = tm;
        }
        for (auto pi : xb) {
            p += pi;
        }
    }
    void delta_conf(std::list<int> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    void output_func(std::list<int> &yb) { yb.push_back(1); }
    double ta() {
        if (p > 0) {
            return sigma;
        } else {
            return DBL_MAX;
        }
    }

    double getSigma() const { return sigma; }
    int getParts() const { return p; }

  private:
    double const tm;  // Machining time
    int p;            // Number of parts in the bin
    double sigma;     // Time to the next output
};

#endif
