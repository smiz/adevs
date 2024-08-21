#ifndef _observer_h_
#define _observer_h_
#include <fstream>
#include "Customer.h"
#include "adevs/adevs.h"

/**
 * The Observer records performance statistics for a Clerk model
 * based on its observable output.
 */
class Observer : public adevs::Atomic<IO_Type> {
  public:
    /// Input port for receiving customers that leave the store.
    static int const departed;
    /// Constructor. Results are written to the specified file.
    Observer(char const* results_file);
    /// Internal transition function.
    void delta_int();
    /// External transition function.
    void delta_ext(double e, adevs::Bag<IO_Type> const &xb);
    /// Confluent transition function.
    void delta_conf(adevs::Bag<IO_Type> const &xb);
    /// Time advance function.
    double ta();
    /// Output function.
    void output_func(adevs::Bag<IO_Type> &yb);

  private:
    /// File for storing information about departing customers.
    std::ofstream output_strm;
};

#endif
