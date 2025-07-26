#ifndef _clerk2_h_
#define _clerk2_h_

#include <list>
#include "Customer.h"
#include "adevs/adevs.h"

class Clerk2 : public adevs::Atomic<EventType> {

  public:
    Clerk2() : Atomic<EventType>(), preempt(0.0), t(0.0) {}

    void delta_int();
    void delta_ext(double e, list<EventType> const &xb);
    void delta_conf(list<EventType> const &xb);
    double ta();
    void output_func(list<EventType> &yb);

    /// Model input port.
    static int const arrive;
    /// Model output port.
    static int const depart;

  private:
    /// Structure for storing information about customers in the line
    struct customer_info_t {
        // The customer
        shared_ptr<Customer> customer;
        // Time remaining to process the customer order
        double t_left;
    };
    /// List of waiting customers.
    std::list<customer_info_t> line;
    //// Time before we can preempt another customer
    double preempt;
    /// The clerk's clock
    double t;
    /// Threshold correspond to a 'small' order processing time
    static double const SMALL_ORDER;
    /// Minimum time between preemptions.
    static double const PREEMPT_TIME;
};

#endif
