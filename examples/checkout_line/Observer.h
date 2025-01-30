#ifndef _observer_h_
#define _observer_h_

#include <fstream>

#include "Customer.h"
#include "adevs/adevs.h"

/**
 * The Observer records performance statistics for a Clerk model
 * based on its observable output.
 */
class Observer : public adevs::Atomic<EventType> {
  public:
    /// Input port for receiving customers that leave the store.
    static int const departed;

    Observer(char const* results_file);
    ~Observer();

    void delta_int();
    void delta_ext(double e, list<EventType> const &xb);
    void delta_conf(list<EventType> const &xb);
    double ta();
    void output_func(list<EventType> &yb);

  private:
    /// File for storing information about departing customers.
    std::ofstream output;
};

#endif
