#ifndef _cell_h_
#define _cell_h_
#include "adevs/adevs.h"

struct value_t {
    bool value;
    int pos;
};

/// A single cell
class Cell : public adevs::Atomic<adevs::CellEvent<value_t>> {
  public:
    Cell(int position, bool const* const state, double h);
    // State transition functions
    void delta_int();
    void delta_ext(double e, list<adevs::CellEvent<value_t>> const &xb);
    void delta_conf(list<adevs::CellEvent<value_t>> const &xb);
    double ta();

    void output_func(list<adevs::CellEvent<value_t>> &yb);

    bool getState() { return q; }
    static void setParams(bool* vis, unsigned rule, int length);

  private:
    static bool* vis;
    static unsigned rule;
    static int length;
    // location of the cell in the 2D space.
    int const pos, left, right;
    double const h;
    // Current cell state
    bool q;
    // Neighboring state
    bool n[2];
    // Time to next event
    double dt;
};

#endif
