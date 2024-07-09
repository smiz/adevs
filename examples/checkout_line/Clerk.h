#ifndef __clerk_h_
#define __clerk_h_
#include <list>
#include "Customer.h"
#include "adevs/adevs.h"

/**
 * The Clerk class is derived from the adevs Atomic class.
 * The Clerk's input/output type is specified using the template
 * parameter of the base class.
 */
class Clerk : public adevs::Atomic<IO_Type> {
  public:
    /// Constructor.
    Clerk();
    /// Internal transition function.
    void delta_int();
    /// External transition function.
    void delta_ext(double e, adevs::Bag<IO_Type> const &xb);
    /// Confluent transition function.
    void delta_conf(adevs::Bag<IO_Type> const &xb);
    /// Output function.
    void output_func(adevs::Bag<IO_Type> &yb);
    /// Time advance function.
    double ta();
    /// Output value garbage collection.
    void gc_output(adevs::Bag<IO_Type> &g);
    /// Destructor.
    ~Clerk();
    /// Model input port.
    static int const arrive;
    /// Model output port.
    static int const depart;

  private:
    /// The clerk's clock
    double t;
    /// List of waiting customers.
    std::list<Customer*> line;
    /// Time spent so far on the customer at the front of the line
    double t_spent;
};

#endif
