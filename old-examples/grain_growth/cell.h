#ifndef _cell_h_
#define _cell_h_
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

    static int angle[SIZE][SIZE];

    Cell(int x, int y);

    void delta_int();
    void delta_ext(double e, std::list<CellEvent> const &xb);
    void delta_conf(std::list<CellEvent> const &xb);

    void output_func(std::list<CellEvent> &yb);

    double ta();

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
