/**
 * Simulation of the system of equations
 *
 * dA/dt = -2AB
 * dB/dt = -AB
 * dC/dt =  AB
 *
 * with a first order QSS method.
 */
#include <iostream>
#include "adevs/adevs.h"
using namespace adevs;
using namespace std;

static double sign(double x) {
    if (x > 0.0) {
        return 1.0;
    } else if (x < 0.0) {
        return -1.0;
    } else {
        return 0.0;
    }
}

class qss : public Atomic<double> {
  public:
    qss(int species)
        : Atomic<double>(),
          species(species),
          count(0),
          ql(y[species]),
          q(y[species]),
          dqdx(0.0) {}
    void init() { dqdx = der(species); }
    void delta_int() {
        count++;
        q = ql = y[species];
        dqdx = der(species);
    }
    void delta_ext(double e, Bag<double> const &x) {
        q += dqdx * e;
        dqdx = der(species);
    }
    void delta_conf(Bag<double> const &x) { delta_int(); }
    void output_func(Bag<double> &yb) {
        y[species] = ql + sign(dqdx) * Q;
        yb.push_back(y[species]);
    }
    double ta() {
        if (dqdx > 0.0) {
            return ((ql + Q) - q) / dqdx;
        } else if (dqdx < 0.0) {
            return ((ql - Q) - q) / dqdx;
        } else {
            return adevs_inf<double>();
        }
    }
    void gc_output(Bag<double> &) {}

    double get_y() const { return y[species]; }
    int get_count() const { return count; }

  private:
    static double const Q;
    static double y[3];
    int const species;
    int count;
    double ql, q, dqdx;

    double der(int species) {
        double dd, A = y[0], B = y[1], C = y[2];
        if (species == 0)  // A
        {
            A = q;
            dd = -2.0 * A * B;
        } else if (species == 1)  // B
        {
            B = q;
            dd = -A * B;
        } else  // C
        {
            C = q;
            dd = 3.0 * A * B;
        }
        return dd;
    }
};

double const qss::Q = 0.01;
double qss::y[3] = {1.0, 1.0, 0.0};

int main() {
    SimpleDigraph<double>* mixture = new SimpleDigraph<double>();
    qss** p = new qss*[3];
    for (int i = 0; i < 3; i++) {
        p[i] = new qss(i);
    }
    for (int i = 0; i < 3; i++) {
        p[i]->init();
    }
    for (int i = 0; i < 3; i++) {
        mixture->add(p[i]);
    }
    mixture->couple(p[0], p[1]);  // A->B
    mixture->couple(p[1], p[0]);  // B->A
    mixture->couple(p[0], p[2]);  // A->C
    mixture->couple(p[1], p[2]);  // B->C
    double tL = 0.0;
    Simulator<double>* sim = new Simulator<double>(mixture);
    while (tL < 1000.0) {
        cout << tL << " ";
        for (int i = 0; i < 3; i++) {
            cout << p[i]->get_y() << " ";
        }
        for (int i = 0; i < 3; i++) {
            cout << p[i]->get_count() << " ";
        }
        cout << endl;
        tL = sim->nextEventTime();
        sim->execNextEvent();
    }
    delete sim;
    delete mixture;
    delete[] p;
    return 0;
}
