#ifndef _proc_h_
#define _proc_h_
#include <cstdlib>
#include "adevs/adevs.h"
#include "job.h"
/*
A processor requires a fixed period of time to service a job.
The processor can serve only one job at a time.  It the processor
is busy, it simply discards incoming jobs.
*/
class proc : public adevs::Atomic<job> {
  public:
    /// Constructor.  The processing time is provided as an argument.
    proc(double proc_time)
        : adevs::Atomic<job>(),
          processing_time(proc_time),
          sigma(adevs_inf<double>()),
          val(nullptr) {
        t = 0.0;
    }
    /// Internal transition function
    void delta_int() {
        t += sigma;
        // Done with the job, so set time of next event to infinity
        sigma = adevs_inf<double>();
        // Discard the completed job
        if (val != nullptr) {
            delete val;
        }
        val = nullptr;
    }
    /// External transition function
    void delta_ext(double e, std::list<PortValue> const &x) {
        t += e;
        // If we are waiting for a job
        if (sigma == adevs_inf<double>()) {
            // Make a copy of the job (original will be destroyed by the
            // generator at the end of this simulation cycle).
            val = new job((*(x.begin())).value);
            // Wait for the required processing time before outputting the
            // completed job
            sigma = processing_time;
        }
        // Otherwise, model just continues with time of next event unchanged
        else {
            sigma -= e;
        }
    }
    /// Confluent transition function.
    void delta_conf(std::list<PortValue> const &x) {
        // Discard the old job
        delta_int();
        // Process the incoming job
        delta_ext(0.0, x);
    }
    /// Output function.
    void output_func(std::list<PortValue> &y) {
        // Produce a copy of the completed job on the out port
        PortValue pv(out, *val);
        y.push_back(pv);
    }
    /// Time advance function.
    double ta() { return sigma; }

    /// Destructor
    ~proc() {
        if (val != nullptr) {
            delete val;
        }
    }

    /// Model input port
    const adevs::pin_t in;
    /// Model output port
    const adevs::pin_t out;

  private:
    /// Model state variables
    double processing_time, sigma;
    job* val;
    double t;
};

#endif
