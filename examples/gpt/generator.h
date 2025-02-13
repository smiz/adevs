#ifndef _generator_h_
#define _generator_h_

#include "adevs/adevs.h"
#include "job.h"

/*
 * The genr class produces jobs periodically.  The genr starts
 * producing jobs when it receives an input on its start port.
 * It stops producing jobs when it receives an input on its
 * stop port.  Jobs appear on the out port.
 */
class Generator : public adevs::Atomic<PortValue> {
  public:
    Generator(double period)
        : adevs::Atomic<PortValue>(), period(period), sigma(period), count(0) {}

    void delta_int() {
        /*
         * We just produced a job via the output_func, so increment the
         * job counter.
         * */
        count++;
        // Wait until its time to produce the next job
        sigma = period;
    }

    void delta_ext(double e, list<PortValue> const &x) {
        // Continue with next event time unchanged if, for some reason,
        // the input is on neither on these ports.
        sigma -= e;
        // Look for input on the start port.  If input is found,
        // hold until it is time to produce the first output.
        list<PortValue>::const_iterator iter;
        for (iter = x.begin(); iter != x.end(); iter++) {
            if ((*iter).port == start) {
                sigma = period;
            }
        }
        // Look for input on the stop port.  If input is found,
        // stop the generator by setting our time of next event to infinity.
        for (iter = x.begin(); iter != x.end(); iter++) {
            if ((*iter).port == stop) {
                sigma = DBL_MAX;
            }
        }
    }

    void delta_conf(list<PortValue> const &x) {
        // When an internal and external event coincide, compute
        // the internal state transition then process the input.
        delta_int();
        delta_ext(0.0, x);
    }

    void output_func(list<PortValue> &y) {
        // Place a new job on the output port
        Job j(count);
        PortValue pv(out, j);
        y.push_back(pv);
    }

    double ta() { return sigma; }


    /// Model input ports
    static int const start;
    static int const stop;
    /// Model output port
    static int const out;

  private:
    /// Model state variables
    double period, sigma;
    int count;
};

/// Create the static ports and assign them unique 'names' (numbers)
int const Generator::stop(0);
int const Generator::start(1);
int const Generator::out(2);

#endif
