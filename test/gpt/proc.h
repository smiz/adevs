#ifndef __proc_h_
#define __proc_h_
#include <cstdlib>
#include "adevs/adevs.h"
#include "job.h"
/*
A processor requires a fixed period of time to service a job.
The processor can serve only one job at a time.  It the processor
is busy, it simply discards incoming jobs.
*/
class proc : public adevs::Atomic<PortValue> {
  public:
    /// Constructor.  The processing time is provided as an argument.
    proc(double proc_time)
        : adevs::Atomic<PortValue>(),
          processing_time(proc_time),
          sigma(DBL_MAX),
          val(NULL) {
        t = 0.0;
    }
    /// Internal transition function
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
    /// External transition function
    void delta_ext(double e, adevs::Bag<PortValue> const &x) {
        t += e;
        // If we are waiting for a job
        if (sigma == DBL_MAX) {
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
    void delta_conf(adevs::Bag<PortValue> const &x) {
        // Discard the old job
        delta_int();
        // Process the incoming job
        delta_ext(0.0, x);
    }
    /// Output function.
    void output_func(adevs::Bag<PortValue> &y) {
        // Produce a copy of the completed job on the out port
        PortValue pv(out, *val);
        y.push_back(pv);
    }
    /// Time advance function.
    double ta() { return sigma; }
    /**
		Garbage collection. No heap allocation in output_func, so
		do nothing.
		*/
    void gc_output(adevs::Bag<PortValue> &) {}
    /// Destructor
    ~proc() {
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
    job* val;
    double t;
};

/// Create unique 'names' for the model ports.
int const proc::in(0);
int const proc::out(1);

#endif
