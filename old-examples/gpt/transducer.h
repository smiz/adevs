#ifndef _transducer_h_
#define _transducer_h_

#include <cstdlib>
#include <iostream>
#include <vector>

#include "adevs/adevs.h"
#include "job.h"

/*
The transducer computes various statistics about the
performance of the queuing system.  It receives new jobs
on its ariv port, finished jobs on its solved port,
and generates and output on its out port when the observation
interval has elapsed.
*/
class Transducer : public adevs::Atomic<PortValue> {
  public:
    Transducer(double observ_time)
        : adevs::Atomic<PortValue>(),
          observation_time(observ_time),
          sigma(observation_time),
          total_ta(0.0),
          t(0.0) {}

    void delta_int() {
        // Keep track of the simulation time
        t += sigma;
        /// Model dumps interesting statistics to the console
        /// when the observation period ends.
        double throughput;
        double avg_ta_time;
        if (!jobs_solved.empty()) {
            avg_ta_time = total_ta / jobs_solved.size();
            if (t > 0.0) {
                throughput = jobs_solved.size() / t;
            } else {
                throughput = 0.0;
            }
        } else {
            avg_ta_time = 0.0;
            throughput = 0.0;
        }
        std::cout << "End time: " << t << std::endl;
        std::cout << "jobs arrived : " << jobs_arrived.size() << std::endl;
        std::cout << "jobs solved : " << jobs_solved.size() << std::endl;
        std::cout << "AVERAGE TA = " << avg_ta_time << std::endl;
        std::cout << "THROUGHPUT = " << throughput << std::endl;
        // Passivate once the data collection period has ended and the
        // results are reported.
        sigma = DBL_MAX;
    }

    void delta_ext(double e, std::list<PortValue> const &x) {
        // Keep track of the simulation time
        t += e;
        // Save new jobs in order to compute statistics when they are
        // completed.
        std::list<PortValue>::iterator iter;
        for (auto iter : x) {
            if (iter.port == ariv) {
                Job j(iter.value);
                j.t = t;
                std::cout << "Start job " << j.id << " @ t = " << t
                          << std::endl;
                jobs_arrived.push_back(j);
            }
        }
        // Compute time required to process completed jobs
        for (auto iter : x) {
            if (iter.port == solved) {
                Job j(iter.value);
                std::vector<Job>::iterator i = jobs_arrived.begin();
                for (; i != jobs_arrived.end(); i++) {
                    if ((*i).id == j.id) {
                        total_ta += t - (*i).t;
                        std::cout << "Finish job " << j.id << " @ t = " << t
                                  << std::endl;
                        break;
                    }
                }
                j.t = t;
                jobs_solved.push_back(j);
            }
        }
        // Continue with next event time unchanged
        sigma -= e;
    }

    void delta_conf(std::list<PortValue> const &x) {
        delta_int();
        delta_ext(0.0, x);
    }

    void output_func(std::list<PortValue> &y) {
        /// Generate an output event to stop the generator
        Job j;
        PortValue pv(out, j);
        y.push_back(pv);
    }

    double ta() { return sigma; }

    ~Transducer() {
        jobs_arrived.clear();
        jobs_solved.clear();
    }

    /// Model input port
    static int const ariv;
    static int const solved;
    /// Model output port
    static int const out;

  private:
    /// Model state variables
    std::vector<Job> jobs_arrived;
    std::vector<Job> jobs_solved;
    double observation_time, sigma, total_ta, t;
};

/// Assign unique 'names' to ports
int const Transducer::ariv(0);
int const Transducer::solved(1);
int const Transducer::out(2);

#endif
