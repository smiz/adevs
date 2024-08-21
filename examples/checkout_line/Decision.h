#ifndef __decision_h_
#define __decision_h_
#include <list>
#include "Customer.h"
#include "adevs/adevs.h"

// Number of lines to consider.
#define NUM_LINES 2

class Decision : public adevs::Atomic<IO_Type> {
  public:
    /// Constructor.
    Decision();
    /// Internal transition function.
    void delta_int();
    /// External transition function.
    void delta_ext(double e, adevs::Bag<IO_Type> const &x);
    /// Confluent transition function.
    void delta_conf(adevs::Bag<IO_Type> const &x);
    /// Output function.
    void output_func(adevs::Bag<IO_Type> &y);
    /// Time advance function.
    double ta();
    /// Input port that receives new customers
    static int const decide;
    /// Input ports that receive customers leaving the two lines
    static int const departures[NUM_LINES];
    /// Output ports that produce customers for the two lines
    static int const arrive[NUM_LINES];

  private:
    /// Lengths of the two lines
    int line_length[NUM_LINES];
    /// List of deciding customers and their decision.
    std::list<std::pair<int, Customer*>> deciding;
    /// Delete all waiting customers and clear the list.
    void clear_deciders();
    /// Returns the arrive port associated with the shortest line
    int find_shortest_line();
};

#endif
