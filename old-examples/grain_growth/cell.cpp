#include "cell.h"
#include <cassert>
#include <climits>
#include <iostream>


int Cell::state_changes = 0;
default_random_engine Cell::generator;
exponential_distribution<double> Cell::exp_dist(W);
uniform_int_distribution<int> Cell::binary_dist(0, 1);
uniform_int_distribution<int> Cell::eight_dist(0, 7);

int Cell::angle[SIZE][SIZE];

static int neighbors[8][2] = {{0, 1}, {0, -1}, {1, 0},  {-1, 0},
                              {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

static bool inside(int x, int y) {
    return x >= 0 && x < SIZE && y >= 0 && y < SIZE;
}

int Cell::energy(int C) {
    int E = 0;
    for (int i = 0; i < 8; i++) {
        int dx = neighbors[i][0] + 1;
        int dy = neighbors[i][1] + 1;
        E += (q[dx][dy] != -1 && C != q[dx][dy]);
    }
    return E;
}

void Cell::set_time_advance() {
    // Is it possible to achieve an equal or lower energy
    // by accepting a neighboring state?
    int E = energy(q[1][1]);
    for (int i = 0; i < 8; i++) {
        int dx = neighbors[i][0] + 1;
        int dy = neighbors[i][1] + 1;
        if (q[dx][dy] != q[1][1] && q[dx][dy] != -1 && energy(q[dx][dy]) <= E) {
            // Yes it is! Leave time of next event unchanged
            // or pick a new one if we were passive
            if (h == adevs_inf<double>()) {
                h = exp_dist(generator);
            }
            return;
        }
    }
    // Nope, its not. We are passive.
    h = adevs_inf<double>();
}

Cell::Cell(int x, int y)
    : adevs::Atomic<CellEvent>(), x(x), y(y), h(adevs_inf<double>()) {
    state_changes++;
    // Record our and the neighboring states
    for (int i = 0; i < 8; i++) {
        int dx = neighbors[i][0] + 1;
        int dy = neighbors[i][1] + 1;
        int px = x + neighbors[i][0];
        int py = y + neighbors[i][1];
        if (inside(px, py)) {
            q[dx][dy] = angle[px][py];
        } else {
            q[dx][dy] = -1;
        }
    }
    q[1][1] = angle[x][y];
    // Set our initial time advance
    set_time_advance();
}

double Cell::ta() {
    return h;
}

void Cell::delta_int() {
    state_changes++;
    // Record our selected output value as the new state
    q[1][1] = angle[x][y];
    // Set the time advance
    h = adevs_inf<double>();
    set_time_advance();
}

void Cell::delta_ext(double e, std::list<CellEvent> const &xb) {
    state_changes++;
    // Update time to next event
    if (h < adevs_inf<double>()) {
        h -= e;
    }
    // Record new neighboring states
    for (auto xx : xb) {
        int dx = xx.value.x_origin - x;
        int dy = xx.value.y_origin - y;
        q[dx + 1][dy + 1] = xx.value.q;
    }
    // Are we passive?
    set_time_advance();
}

void Cell::delta_conf(std::list<CellEvent> const &xb) {
    state_changes++;
    // Record our selected output value as the new state
    q[1][1] = angle[x][y];
    // Record new neighboring states
    for (auto xx : xb) {
        int dx = xx.value.x_origin - x;
        int dy = xx.value.y_origin - y;
        q[dx + 1][dy + 1] = xx.value.q;
    }
    // Set the time advance
    h = adevs_inf<double>();
    set_time_advance();
}

void Cell::output_func(std::list<CellEvent> &yb) {
    // What will our new choice be?
    int pick, dx, dy;
    do {
        pick = eight_dist(generator);
        dx = neighbors[pick][0] + 1;
        dy = neighbors[pick][1] + 1;
    } while (q[dx][dy] == -1);
    // No change, so don't bother reporting it
    if (q[dx][dy] == angle[x][y]) {
        return;
    }
    // Compare energy levels. If we accept the change, report it
    int E = energy(angle[x][y]);
    int En = energy(q[dx][dy]);
    if (En < E || (En == E && binary_dist(generator) == 0)) {
        CellEvent e;
        e.value.q = angle[x][y] = q[dx][dy];
        e.value.x_origin = x;
        e.value.y_origin = y;
        // Produce an event for each of the 8 neighbors
        for (int i = 0; i < 8; i++) {
            e.x = x + neighbors[i][0];
            e.y = y + neighbors[i][1];
            yb.push_back(e);
        }
    }
}
