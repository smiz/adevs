#ifndef _Machine_h_
#define _Machine_h_
#include <cassert>
#include <deque>
#include "adevs/adevs.h"

using Atomic = adevs::Atomic<int>;

/**
 * This class models a machine as a fifo queue and server with fixed service time.
 * The model_transition method is used, in conjunction with the Factory model_transition
 * method, to add and remove machines as needed to satisfy a 6 day turnaround time
 * for orders.
 */
class Machine : public Atomic {
  public:
    Machine() : Atomic(), time_remaining(DBL_MAX) {}

    void delta_int() {
        q.pop_front();  // Remove the completed job
        if (q.empty()) {
            time_remaining = DBL_MAX;  // Is the Machine idle?
        } else {
            time_remaining = 3.0;  // Or is it still working?
        }
    }

    void delta_ext(double e, std::list<int> const &xb) {
        // Update the remaining time if the machine is working
        if (!q.empty()) {
            time_remaining -= e;
        }
        // Put new orders into the queue
        std::list<int>::const_iterator iter = xb.begin();
        for (; iter != xb.end(); iter++) {
            // If the machine is idle then set the service time
            if (q.empty()) {
                time_remaining = 3.0;
            }
            // Put the order into the back of the queue
            q.push_back(*iter);
        }
    }

    void delta_conf(std::list<int> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }

    void output_func(std::list<int> &yb) {
        // Expel the completed order
        yb.push_back(q.front());
    }

    double ta() { return time_remaining; }

    // The model transition function returns true if another order can not
    // be accomodated or if the machine is idle.
    bool model_transition() {
        // Check that the queue size is legal
        assert(q.size() <= 2);
        // Return the idle or full status
        return (q.size() == 0 || q.size() == 2);
    }

    // Get the number of orders in the queue
    unsigned int getQueueSize() const { return q.size(); }

  private:
    // Queue for orders that are waiting to be processed
    std::deque<int> q;

    double time_remaining;
};

#endif
