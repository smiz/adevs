#ifndef _clerk2_h_
#define _clerk2_h_
#include <list>
#include "Customer.h"
#include "adevs/adevs.h"

class Clerk2 : public adevs::Atomic<IO_Type> {
  public:
    /// Constructor.
    Clerk2();
    /// Internal transition function.
    void delta_int();
    /// External transition function.
    void delta_ext(double e, list<IO_Type> const &xb);
    /// Confluent transition function.
    void delta_conf(list<IO_Type> const &xb);
    /// Time advance function.
    double ta();
    /// Output function.
    void output_func(list<IO_Type> &yb);

    /// Model input port.
    static int const arrive;
    /// Model output port.
    static int const depart;

  private:
    /// Structure for storing information about customers in the line
    struct customer_info_t {
        // The customer
        Customer* customer;
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
