#include "Cell.h"

using namespace std;

// Cellspace dimensions
long int Cell::w = 0;
long int Cell::h = 0;


Cell::Cell(long int x, long int y, long int w, long int h, Phase phase,
           short int nalive, Phase* vis_phase)
    : adevs::Atomic<CellEvent, int>(),
      x(x),
      y(y),
      phase(phase),
      nalive(nalive),
      vis_phase(vis_phase) {
    // Set the global cellspace dimensions
    Cell::w = w;
    Cell::h = h;
    // Set the initial visualization value
    if (vis_phase != NULL) {
        *vis_phase = phase;
    }
}

int Cell::ta() {
    // Check if a phase change should occur
    if (check_death_rule()     // cell will die
        || check_born_rule())  // cell will be birthed
    {
        return 1;
    }
    // Otherwise, do nothing
    return adevs_inf<int>();
}

void Cell::delta_int() {
    // One of these must be true
    assert(check_death_rule() || check_born_rule());
    // Change our state
    phase = (check_death_rule()) ? Dead : Alive;
}

void Cell::delta_ext(int e, list<CellEvent> const &xb) {
    // Update the count of living neighbors
    list<CellEvent>::const_iterator iter;
    for (iter = xb.begin(); iter != xb.end(); iter++) {
        if ((*iter).value == Dead) {
            nalive--;
        } else {
            nalive++;
        }
    }
}

void Cell::delta_conf(list<CellEvent> const &xb) {
    delta_int();
    delta_ext(0, xb);
}

void Cell::output_func(list<CellEvent> &yb) {
    CellEvent e;
    // Out is the state that we will assume
    e.value = (check_born_rule()) ? Alive : Dead;
    // Set the visualization value
    if (vis_phase != NULL) {
        *vis_phase = e.value;
    }
    // Generate an event for each neighbor
    for (long int dx = -1; dx <= 1; dx++) {
        for (long int dy = -1; dy <= 1; dy++) {
            e.x = (x + dx) % w;
            e.y = (y + dy) % h;
            if (e.x < 0) {
                e.x = w - 1;
            }
            if (e.y < 0) {
                e.y = h - 1;
            }
            // Don't send to self
            if (e.x != x || e.y != y) {
                yb.push_back(e);
            }
        }
    }
}
