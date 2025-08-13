#ifndef _generator_h_
#define _generator_h_

#include <list>
#include "Customer.h"
#include "adevs/adevs.h"

/**
 * This class produces Customers according to the provided schedule.
 */
class Generator : public adevs::Atomic<EventType> {

  public:
    Generator(std::string data_file);

    void delta_int();
    void delta_ext(double e, list<EventType> const &xb);
    void delta_conf(list<EventType> const &xb);
    void output_func(list<EventType> &yb);
    double ta();

    /// Model output port.
    static int const arrive;

  private:
    /// List of arriving customers.
    std::list<std::shared_ptr<Customer>> arrivals;
};

#endif
