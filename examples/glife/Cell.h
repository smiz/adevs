#ifndef _cell_h_
#define _cell_h_
#include "adevs/adevs.h"

/// Possible cell phases.
typedef enum { Dead, Alive } Phase;
/// IO type for a cell
typedef adevs::CellEvent<Phase> CellEvent;

/// A cell in the Game of Life. This uses an integer time base.
class Cell : public adevs::Atomic<CellEvent, int> {
  public:
    /**
    Create a cell and set the initial state.
    The width and height fields are used to determine if a
    cell is an edge cell.  The last phase pointer is used to
    visualize the cell space.
    */
    Cell(long int x, long int y, long int width, long int height, Phase phase,
         short int nalive, Phase* vis_phase = NULL);

    void delta_int();
    void delta_ext(int e, list<CellEvent> const &xb);
    void delta_conf(list<CellEvent> const &xb);

    int ta();
    void output_func(list<CellEvent> &yb);

    ~Cell() {}

  private:
    // location of the cell in the 2D space.
    long int x, y;
    // dimensions of the 2D space
    static long int w, h;
    // Current cell phase
    Phase phase;
    // number of living neighbors.
    short int nalive;
    // Output variable for visualization
    Phase* vis_phase;

    // Returns true if the cell will be born
    bool check_born_rule() const { return (phase == Dead && nalive == 3); }
    // Return true if the cell will die
    bool check_death_rule() const {
        return (phase == Alive && (nalive < 2 || nalive > 3));
    }
};

#endif
