#ifndef _Machine_h_
#define _Machine_h_
#include <cassert>
#include <deque>
#include "adevs/adevs.h"
/**
 * This class models a machine as a fifo queue and server with fixed service time.
 * The model_transition method is used, in conjunction with the Factory model_transition
 * method, to add and remove machines as needed to satisfy a 6 day turnaround time
 * for orders.
 */
class Machine : public adevs::Atomic<int> {
  public:
    Machine() : adevs::Atomic<int>(), tleft(DBL_MAX) {}
    void delta_int() {
        q.pop_front();  // Remove the completed job
        if (q.empty()) {
            tleft = DBL_MAX;  // Is the Machine idle?
        } else {
            tleft = 3.0;  // Or is it still working?
        }
    }
    void delta_ext(double e, adevs::Bag<int> const &xb) {
        // Update the remaining time if the machine is working
        if (!q.empty()) {
            tleft -= e;
        }
        // Put new orders into the queue
        adevs::Bag<int>::const_iterator iter = xb.begin();
        for (; iter != xb.end(); iter++) {
            // If the machine is idle then set the service time
            if (q.empty()) {
                tleft = 3.0;
            }
            // Put the order into the back of the queue
            q.push_back(*iter);
        }
    }
    void delta_conf(adevs::Bag<int> const &xb) {
        delta_int();
        delta_ext(0.0, xb);
    }
    void output_func(adevs::Bag<int> &yb) {
        // Expel the completed order
        yb.push_back(q.front());
    }
    double ta() { return tleft; }
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
    // Time remaining on the order at the front of the queue
    double tleft;
};

#endif
