#ifndef _job_h_
#define _job_h_

#include "adevs/adevs.h"

/*
 * A job to be processed by a processor.  The job has a timestamp field
 * that is used to compute thruput, turnaround, etc, etc.  It has an
 * integer value which is the job ID.
 */
struct Job {
    // Time of job creation
    int id;
    double t;

    Job() : id(0), t(0.0) {}
    Job(int x) : id(x), t(0.0) {}
    Job(Job const &src) : id(src.id), t(src.t) {}
    Job const &operator=(Job const &src) {
        id = src.id;
        t = src.t;
        return *this;
    }
};

typedef adevs::PortValue<Job> PortValue;

#endif
