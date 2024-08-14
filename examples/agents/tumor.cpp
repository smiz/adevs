/**
 * This is an agent based simulation of a cancerous tumor.
 * Output is printed to the console.
 */
#include <cmath>
#include <iostream>
#include <set>
#include <vector>
#include "adevs/adevs.h"
using namespace std;
using namespace adevs;

#define XSIZE 10
#define YSIZE 10
// Status of the cells in the space
static char output[XSIZE][YSIZE];
// Real time between divisions of a cell
static double const r = 2.0;
// A healthy site
static char const Healthy = ' ';
// A tumorous site
static char const Tumor = 't';
// A necrotic site
static char const Necrotic = 'n';

struct io_type {
    // Source cell
    int xsrc, ysrc, xtgt, ytgt;
};

class Agent : public Atomic<io_type, sd_time<>> {
  public:
    Agent(int c0, int p, int x, int y)
        : Atomic<io_type, sd_time<>>(), c(c0), p(p), x(x), y(y) {
        output[x][y] = c0;
    }
    void delta_int() {}
    void delta_ext(sd_time<> e, Bag<io_type> const &xb) {
        for (auto xx : xb) {
            if (xx.xtgt == x && xx.ytgt == y && c == Healthy &&
                output[xx.xsrc][xx.ysrc] == Tumor) {
                c = Tumor;
            }
        }
        check_neighbors();
    }
    void delta_conf(Bag<io_type> const &xb) {
        delta_int();
        delta_ext(adevs_zero<sd_time<>>(), xb);
    }
    void output_func(Bag<io_type> &yb) {
        io_type yy;
        yy.xsrc = x;
        yy.ysrc = y;
        yy.xtgt = x;
        yy.ytgt = y;
        int select = rand() % 4;
        if (select == 0) {
            yy.xtgt++;
        } else if (select == 1) {
            yy.ytgt++;
        } else if (select == 2) {
            yy.xtgt--;
        } else {
            yy.ytgt--;
        }
        if (yy.ytgt < 0) {
            yy.ytgt = YSIZE - 1;
        } else if (yy.ytgt == YSIZE) {
            yy.ytgt = 0;
        }
        if (yy.xtgt < 0) {
            yy.xtgt = XSIZE - 1;
        } else if (yy.xtgt == XSIZE) {
            yy.xtgt = 0;
        }
        yb.push_back(yy);
        output[x][y] = c;
    }
    void gc_output(Bag<io_type> &) {}
    sd_time<> ta() {
        if (output[x][y] != c) {
            return sd_time<>(0.0, 0);
        } else if (c == Tumor) {
            return sd_time<>(r, p);
        } else {
            return adevs_inf<sd_time<>>();
        }
    }

  private:
    char c;
    int const p, x, y;

    void check_neighbors() {
        int count = 0;
        int xx = x;
        int yy = y;
        if (xx - 1 < 0 && output[XSIZE - 1][y] != Healthy) {
            count++;
        } else if (xx - 1 >= 0 && output[xx - 1][y] != Healthy) {
            count++;
        }
        if (yy - 1 < 0 && output[x][YSIZE - 1] != Healthy) {
            count++;
        } else if (yy - 1 >= 0 && output[x][yy - 1] != Healthy) {
            count++;
        }
        if (xx + 1 == XSIZE && output[0][y] != Healthy) {
            count++;
        } else if (xx + 1 < XSIZE && output[xx + 1][y] != Healthy) {
            count++;
        }
        if (yy + 1 == YSIZE && output[x][0] != Healthy) {
            count++;
        } else if (yy + 1 < YSIZE && output[x][yy + 1] != Healthy) {
            count++;
        }
        if (count == 4 && c == Tumor) {
            c = Necrotic;
        }
    }
};

class Grid : public Network<io_type, sd_time<>> {
  public:
    Grid() : Network<io_type, sd_time<>>() {
        int count = 0;
        for (int i = 0; i < XSIZE; i++) {
            for (int j = 0; j < YSIZE; j++) {
                if (i == XSIZE / 2 && j == YSIZE / 2) {
                    grid[i][j] = new Agent(Tumor, count++, i, j);
                } else {
                    grid[i][j] = new Agent(Healthy, count++, i, j);
                }
                grid[i][j]->setParent(this);
            }
        }
    }
    void getComponents(set<Devs<io_type, sd_time<>>*> &c) {
        for (int i = 0; i < XSIZE; i++) {
            for (int j = 0; j < YSIZE; j++) {
                c.insert(grid[i][j]);
            }
        }
    }
    void route(io_type const &value, Devs<io_type, sd_time<>>* model,
               Bag<Event<io_type, sd_time<>>> &r) {
        Event<io_type, sd_time<>> xx;
        xx.value = value;
        if (value.xsrc - 1 >= 0) {
            xx.model = grid[value.xsrc - 1][value.ysrc];
        } else {
            xx.model = grid[XSIZE - 1][value.ysrc];
        }
        r.push_back(xx);
        if (value.xsrc + 1 < XSIZE) {
            xx.model = grid[value.xsrc + 1][value.ysrc];
        } else {
            xx.model = grid[0][value.ysrc];
        }
        r.push_back(xx);
        if (value.ysrc - 1 >= 0) {
            xx.model = grid[value.xsrc][value.ysrc - 1];
        } else {
            xx.model = grid[value.xsrc][YSIZE - 1];
        }
        r.push_back(xx);
        if (value.ysrc + 1 < YSIZE) {
            xx.model = grid[value.xsrc][value.ysrc + 1];
        } else {
            xx.model = grid[value.xsrc][0];
        }
        r.push_back(xx);
    }
    ~Grid() {
        for (int i = 0; i < XSIZE; i++) {
            for (int j = 0; j < YSIZE; j++) {
                delete grid[i][j];
            }
        }
    }
    void print(sd_time<> tL) {
        cout << tL << endl;
        for (int i = 0; i < XSIZE; i++) {
            for (int j = 0; j < YSIZE; j++) {
                cout << output[i][j];
            }
            cout << endl;
        }
        cout << endl;
    }

  private:
    Agent* grid[XSIZE][YSIZE];
};

int main() {
    Grid* world = new Grid();
    Simulator<io_type, sd_time<>>* sim =
        new Simulator<io_type, sd_time<>>(world);
    world->print(adevs_zero<sd_time<>>());
    while (sim->nextEventTime() < adevs_inf<sd_time<>>()) {
        sd_time<> tL = sim->nextEventTime();
        sim->execNextEvent();
        world->print(tL);
    }
    delete sim;
    delete world;
}
