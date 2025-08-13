#ifndef _clerk_h_
#define _clerk_h_

#include <list>
#include <memory>
#include "Customer.h"
#include "adevs/adevs.h"

/**
 * The Clerk class is derived from the adevs Atomic class.
 * The Clerk's input/output type is specified using the template
 * parameter of the base class.
 */
class Clerk : public adevs::Atomic<EventType> {
  public:
    // Initialize the clock to zero with no time spent on a customer so far
    Clerk() : Atomic<EventType>(), t(0.0), time_spent(0.0) {}

    /// Internal transition function.
    void delta_int();
    /// External transition function.
    void delta_ext(double e, list<EventType> const &xb);
    /// Confluent transition function.
    void delta_conf(list<EventType> const &xb);
    /// Output function.
    void output_func(list<EventType> &yb);
    /// Time advance function.
    double ta();

    /// Model input port.
    static int const arrive;

    /// Model output port.
    static int const depart;

  private:
    /// The clerk's clock
    double t = 0.0;

    /// List of waiting customers.
    std::list<std::shared_ptr<Customer>> line;

    /// Time spent so far on the customer at the front of the line
    double time_spent = 0.0;
};

#endif
