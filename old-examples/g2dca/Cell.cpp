#include "Cell.h"
using namespace std;

// Cellspace dimensions
int Cell::w = 0;
int Cell::h = 0;
Phase** Cell::y_space = NULL;
bool Cell::rule[512];

void Cell::init_space(int w, int h, uint8_t const* rule) {
    if (y_space != NULL) {
        for (int i = 0; i < Cell::w; i++) {
            delete[] y_space[i];
        }
        delete[] y_space;
    }
    Cell::w = w;
    Cell::h = h;
    y_space = new Phase*[Cell::w];
    for (int i = 0; i < Cell::w; i++) {
        y_space[i] = new Phase[Cell::h];
    }
    for (int i = 0; i < RULE_ARRAY_LEN; i++) {
        for (int j = 0; j < 8; j++) {
            Cell::rule[4 * i + j] = (rule[i] >> j) & 0x01;
        }
    }
}

Cell::Cell(int x, int y, Phase phase, Phase* vis_phase)
    : adevs::Atomic<CellEvent, int>(),
      x(x),
      y(y),
      phase(phase),
      vis_phase(vis_phase) {
    // Set the initial visualization value
    if (vis_phase != NULL) {
        *vis_phase = phase;
    }
    y_space[x][y] = phase;
}

unsigned Cell::rule_index() {
    unsigned rule = 0, idx = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int xx = x + dx;
            int yy = y + dy;
            if (xx < 0) {
                xx = w - 1;
            } else if (xx >= w) {
                xx = 0;
            }
            if (yy < 0) {
                yy = h - 1;
            } else if (yy >= h) {
                yy = 0;
            }
            if (y_space[xx][yy] == Alive) {
                rule |= (1 << idx);
            }
            idx++;
        }
    }
    return rule;
}

int Cell::ta() {
    bool qn = rule[rule_index()];
    if ((phase == Alive && !qn) || (phase == Dead && qn)) {
        return 1;
    }
    // Otherwise, do nothing
    return adevs_inf<int>();
}

void Cell::delta_int() {
    phase = y_space[x][y];
}

void Cell::delta_ext(int e, list<CellEvent> const &xb) {}

void Cell::delta_conf(list<CellEvent> const &xb) {
    delta_int();
}

void Cell::output_func(list<CellEvent> &yb) {
    CellEvent e;
    // Assume we are dying
    e.value = (rule[rule_index()]) ? Alive : Dead;
    // Set the initial visualization value
    if (vis_phase != NULL) {
        *vis_phase = e.value;
    }
    y_space[x][y] = e.value;
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
