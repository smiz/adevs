#ifndef __cell_h_
#define __cell_h_
#include <adevs/adevs.h>
#include <iostream>
#include <random>
#include "params.h"  // Includes SIZE and W

struct event_t {
    int q;
    int x_origin, y_origin;
};

typedef adevs::CellEvent<event_t> CellEvent;

/**
 * A lattice point in our grain growth model.
 */
class Cell : public adevs::Atomic<CellEvent> {
  public:
    static int state_changes;
    // Random number generators
    static std::default_random_engine generator;
    static std::exponential_distribution<double> exp_dist;
    static std::uniform_int_distribution<int> binary_dist;
    static std::uniform_int_distribution<int> eight_dist;
    // Cell state map
    static int angle[SIZE][SIZE];
    // Create a cell at x,y
    Cell(int x, int y);
    // Internal transition function
    void delta_int();
    // External transition function
    void delta_ext(double e, adevs::Bag<CellEvent> const &xb);
    // Confluent transition function
    void delta_conf(adevs::Bag<CellEvent> const &xb);
    // Output function
    void output_func(adevs::Bag<CellEvent> &yb);
    // Time advance function
    double ta();
    // Garbage collection. Does nothing.
    void gc_output(adevs::Bag<CellEvent> &g) {}
    // Destructor
    ~Cell() {}

  private:
    // Cell location
    int const x, y;
    // Self and surrounding values
    int q[3][3];
    // Time remaining to next grain boundary event
    double h;

    void set_time_advance();
    int energy(int C);
};

#endif
