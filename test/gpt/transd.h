#ifndef _transd_h_
#define _transd_h_
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
class transd : public adevs::Atomic<job> {
  public:
    /// Constructor
    transd(double observ_time)
        : adevs::Atomic<job>(),
          observation_time(observ_time),
          sigma(observation_time),
          total_ta(0.0),
          t(0.0) {}
    /// Internal transition function
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
        sigma = adevs_inf<double>();
    }
    /// External transition function
    void delta_ext(double e, std::list<PortValue> const &x) {
        // Keep track of the simulation time
        t += e;
        // Save new jobs in order to compute statistics when they are
        // completed.
        std::list<PortValue>::iterator iter;
        for (auto iter : x) {
            if (iter.pin == ariv) {
                job j(iter.value);
                j.t = t;
                std::cout << "Start job " << j.id << " @ t = " << t
                          << std::endl;
                jobs_arrived.push_back(j);
            }
        }
        // Compute time required to process completed jobs
        for (auto iter : x) {
            if (iter.pin == solved) {
                job j(iter.value);
                std::vector<job>::iterator i = jobs_arrived.begin();
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
        job j;
        PortValue pv(out, j);
        y.push_back(pv);
    }

    double ta() { return sigma; }

    ~transd() {
        jobs_arrived.clear();
        jobs_solved.clear();
    }
    /// Model input port
    const adevs::pin_t ariv, solved;
    /// Model output port
    const adevs::pin_t out;

  private:
    /// Model state variables
    std::vector<job> jobs_arrived;
    std::vector<job> jobs_solved;
    double observation_time, sigma, total_ta, t;
};

#endif
