#ifndef _generator_h_
#define _generator_h_
#include <list>
#include "Customer.h"
#include "adevs/adevs.h"

/**
 * This class produces Customers according to the provided schedule.
 */
class Generator : public adevs::Atomic<IO_Type> {
  public:
    /// Constructor.
    Generator(char const* data_file);
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
    ~Generator();
    /// Model output port.
    static int const arrive;

  private:
    /// List of arriving customers.
    std::list<Customer*> arrivals;
};

#endif
