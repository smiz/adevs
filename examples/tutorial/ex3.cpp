#include <iostream>
#include <random>
#include "adevs/adevs.h"

using Atomic = adevs::Atomic<>;
using PinValue = adevs::PinValue<>;
using Simulator = adevs::Simulator<>;
using Graph = adevs::Graph<>;
using pin_t = adevs::pin_t;

/**
 * This example is shows how a queuing system can be
 * modeled. It is our first example that exercises every
 * part of the Atomic class interface. An ArrivalProcess
 * produces jobs (e.g., customers arriving at a fast
 * food restaurant). Customers arrive at a rate modeled
 * by a Poisson process, in which the time between the
 * arrival of two customers is exponentially distributed.
 * 
 * The ArrivalProcess produces output that is routed to
 * a Server model. The server contains a queue of jobs
 * that are waiting to be processed (e.g., customers
 * standing in a line). A single job is processed in
 * 1 unit of time. The Server keeps track of how much
 * time each job spends waiting in the Server.
 */

/**
  * Our ArrivalProcess produces output at intervals drawn
  * from an exponentially distributed random number. This
  * random number is returned by the time advance function.
  * When an interval of time equal to the time advance expires,
  * the output function is called to produce a PinValue
  * object that contains a job to be processed by the Server.
  */
class ArrivalProcess : public Atomic {
  public:
    /// Our constructor calls the default constructor and creates
    /// a random number generator from which we will draw arrival
    /// times. The mean interval between arrivals is set to 1.0.
    ArrivalProcess() : Atomic(), interarrival(1.0), generator(std::random_device {}()) {}
    /// Our time to next event is the interval of time that must pass
    /// before the next job arrives.
    double ta() { return interarrival(generator); }
    /// When the time advance expires, we produce a job by placing it
    /// into the list of output values. The value on the pin doesn't
    /// mean anything in this example.
    void output_func(std::list<PinValue> &yb) { yb.push_back(PinValue(job, 0)); }
    /// The internal transition method is called but it doesn't do anything
    /// in this example
    void delta_int() {}
    /// The external transition function is not used in this example.
    /// It is never called by the Simulator.
    void delta_ext(double, std::list<PinValue> const &) {}
    /// The confluent transition function is not used in this example,
    /// It is never called by the Simulator.
    void delta_conf(std::list<PinValue> const &) {}

    /// The single pin on which jobs will appear
    pin_t const job;

  private:
    std::exponential_distribution<> interarrival;
    std::default_random_engine generator;
};

/**
 * The Server has three variables that make up its state.
 * One of these is the list of jobs that is waiting to be
 * processed. The first item in the list is always the 
 * job that we are working on. The next state variable is
 * the amount of time needed to finish that job. Finally,
 * the Server keeps track of the total time that has 
 * elapsed since the start of the simulation and uses this
 * information to calculate how long each job spends in the
 * system, both waiting and being processed.
 */
class Server : public Atomic {
  public:
    /// The time to finish is infinity at the start because we have no
    /// work to do; that is, it will be an infinite amount of time
    /// before we finish a job.
    Server() : Atomic(), time_since_start(0.0), time_to_finish(adevs_inf<double>()) {}
    /// The time advance function returns the time remaining to finish
    /// the current job.
    double ta() { return time_to_finish; }
    /// The output function produces the completed job, even though it
    /// won't be routed anywhere by the Simulator. The value on the
    /// pin doesn't mean anything in this example.
    void output_func(std::list<PinValue> &yb) {
        /// Test your knowledge: why is this assertion always true?
        assert(!job_queue.empty());
        yb.push_back(PinValue(finished_job, 0));
    }
    /// The internal transition function is called by the Simulator
    /// when a job is complete. We pop the job off of the queue and
    /// set the time to finish the next job to 1 if we have a job
    /// or infinity if the queue is empty.
    void delta_int() {
        /// Test your knowledge: why is this assertion always true?
        assert(!job_queue.empty());
        /// Update the time since the Server can into being
        time_since_start += ta();
        /// Report how long this job spent waiting in the queue
        std::cout << time_since_start - job_queue.front() << std::endl;
        /// Remove the job from the queue
        job_queue.pop_front();
        time_to_finish = (job_queue.empty()) ? adevs_inf<double>() : 1.0;
    }
    /// The external transition function is called by the Simulator when
    /// a new job is produced as output by the ArrivalProcess model.
    /// We do two things when this happens.
    ///
    /// First, we use the time elapsed since our previous state change
    /// - that is, the most recent prior call to a delta_int, delta_ext,
    /// or delta_conf method - to update the amount of time needed to
    /// to finish the job we are working on. This elapsed time is the
    /// e parameter in the method signature. If we weren't working any
    /// job then we set the time to complete the new job equal to 1.
    ///
    /// Second, we push the newly arrived jobs to the back of the job queue.
    void delta_ext(double e, std::list<PinValue> const &xb) {
        /// Test your knowledge: why is this assertion always true?
        assert(e < time_to_finish);
        /// Update the total time elapsed since the Server came into being
        time_since_start += e;
        /// We spent e units of time working on the job since the last
        /// call to a state transition function. Account for this in the
        /// time remaining to complete the job we are working on.
        if (job_queue.empty()) {
            time_to_finish = 1.0;
        } else {
            time_to_finish -= e;
        }
        /// Put the new jobs into the back of the queue
        for (auto const &input : xb) {
            job_queue.push_back(time_since_start);
        }
    }
    /// The confluent transition function is never called by the Simulator.
    void delta_conf(std::list<PinValue> const &xb) {
        /// Test your knowledge: why is this assertion always true?
        assert(!job_queue.empty());
        /// Update the time since the Server can into being
        time_since_start += ta();
        /// Report how long this job spent waiting in the queue
        std::cout << time_since_start - job_queue.front() << std::endl;
        /// Remove the job from the queue
        job_queue.pop_front();
        /// Add new jobs to the back of the queue
        for (auto const &input : xb) {
            job_queue.push_back(time_since_start);
        }
        /// We are always starting a new job when the confluent transition
        /// function is activated.
        time_to_finish = 1.0;
    }

    /// Finished jobs are produced on this pin.
    pin_t const finished_job;

  private:
    /// The since the beginning of the simulation
    double time_since_start;
    /// The time remaining to finish the current job.
    double time_to_finish;
    /// Jobs waiting to be processed are stored in a queue. The
    /// jobs are stored using their time of entry into the queue.
    std::list<double> job_queue;
};

/**
 * The main function creates a Simulator for our model and runs
 * the simulation for 10 units of time.
 */
int main() {
    /// Create an instance of the Supplier model in a shared pointer
    auto arrivals = std::make_shared<ArrivalProcess>();
    /// Create an instance of the Consumer model in a shared pointer
    auto server = std::make_shared<Server>();
    /// Create a Graph to connect the two models
    auto graph = std::make_shared<Graph>();
    /// Add the Atomic components to the Graph
    graph->add_atomic(arrivals);
    graph->add_atomic(server);
    /// PinValue objects that appears as output with arrivals->job as
    /// their pin member will be supplied as input to the Server
    graph->connect(arrivals->job, server);
    /// Create a Simulator for the Graph
    Simulator simulator(graph);
    /// Run the simulator for 2 units of time
    while (simulator.nextEventTime() <= 10.0) {
        simulator.execNextEvent();
    }
    /// Done!
    return 0;
}
