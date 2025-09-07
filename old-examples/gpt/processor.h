#ifndef _processor_h_
#define _processor_h_

#include <cstdlib>

#include "adevs/adevs.h"
#include "job.h"

/*
 * A processor requires a fixed period of time to service a job.
 * The processor can serve only one job at a time.  It the processor
 * is busy, it simply discards incoming jobs.
 */
class Processor : public adevs::Atomic<PortValue> {
  public:
    Processor(double proc_time)
        : adevs::Atomic<PortValue>(),
          processing_time(proc_time),
          sigma(DBL_MAX),
          val(NULL) {
        t = 0.0;
    }

    void delta_int() {
        t += sigma;
        // Done with the job, so set time of next event to infinity
        sigma = DBL_MAX;
        // Discard the completed job
        if (val != NULL) {
            delete val;
        }
        val = NULL;
    }

    void delta_ext(double e, std::list<PortValue> const &x) {
        t += e;
        // If we are waiting for a job
        if (sigma == DBL_MAX) {
            // Make a copy of the job (original will be destroyed by the
            // generator at the end of this simulation cycle).
            val = new Job((*(x.begin())).value);
            // Wait for the required processing time before outputting the
            // completed job
            sigma = processing_time;
        }
        // Otherwise, model just continues with time of next event unchanged
        else {
            sigma -= e;
        }
    }

    void delta_conf(std::list<PortValue> const &x) {
        // Discard the old job
        delta_int();
        // Process the incoming job
        delta_ext(0.0, x);
    }

    void output_func(std::list<PortValue> &y) {
        // Produce a copy of the completed job on the out port
        PortValue pv(out, *val);
        y.push_back(pv);
    }

    double ta() { return sigma; }

    ~Processor() {
        if (val != NULL) {
            delete val;
        }
    }

    /// Model input port
    static int const in;
    /// Model output port
    static int const out;

  private:
    /// Model state variables
    double processing_time, sigma;
    Job* val;
    double t;
};

/// Create unique 'names' for the model ports.
int const Processor::in(0);
int const Processor::out(1);

#endif
