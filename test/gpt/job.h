#ifndef _job_h_
#define _job_h_
#include "adevs/adevs.h"

/*
A job to be processed by a processor.  The job has a timestamp field
that is used to compute thruput, turnaround, etc, etc.  It has an
integer value which is the job ID.
*/
struct job {
    /// Time of job creation
    int id;
    double t;

    job() : id(0), t(0.0) {}
    job(int x) : id(x), t(0.0) {}
    job(job const &src) : id(src.id), t(src.t) {}
    job const &operator=(job const &src) {
        id = src.id;
        t = src.t;
        return *this;
    }
};

using PortValue = adevs::PinValue<job>;

#endif
