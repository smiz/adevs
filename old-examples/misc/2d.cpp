/**
 * Solution to dx/dt = Ax, A a 2 x 2 matrix
 */
#include <cstdlib>
#include <iostream>
#include "adevs/adevs.h"
// using namespace adevs;


static double const A[2][2] = {{-2.0, 1.0}, {0.0, -1.0}};

double const x0[2] = {0.0, sqrt(2.0)};

struct event_t {
    int k;
    double x;
};

class qss1 : public Atomic<event_t> {
  public:
    qss1(double q, int i) : Atomic<event_t>(), q(q), i(i), j((i + 1) % 2) {
        xl = xi = x0[i];
        xj = x0[j];
        pick_next(q);
    }
    double ta() { return h; }
    void delta_int() {
        xi = xl = xn;
        pick_next(q);
    }
    void delta_conf(std::list<event_t> const &xb) {
        xj = xb[0].x;
        xi = xl = xn;
        pick_next(q);
    }
    void delta_ext(double e, std::list<event_t> const &xb) {
        double f = A[i][i] * xi + A[i][j] * xj;
        xi += e * f;
        xj = xb[0].x;
        double dx = q - fabs(xi - xl);
        if (dx < 0.0) {
            xn = xi;
            h = 0.0;
        } else {
            pick_next(dx);
        }
    }
    void output_func(std::list<event_t> &y) {
        event_t yy;
        yy.k = i;
        yy.x = xn;
        y.push_back(yy);
    }


  private:
    void pick_next(double D) {
        double f = A[i][i] * xi + A[i][j] * xj;
        if (f == 0) {
            xn = xi;
            h = adevs_inf<double>();
            return;
        }
        D = (f > 0.0) ? D : -D;
        xn = xi + D;
        h = fabs(D / f);
    }
    double const q;
    int const i, j;
    double xj, xi, xl, h, xn;
};

class qss2 : public Atomic<event_t> {
  public:
    qss2(double q, int i) : Atomic<event_t>(), q(q), i(i), j((i + 1) % 2) {
        xl = xi = x0[i];
        xj = x0[j];
        pick_next(q);
    }
    double ta() { return h; }
    void delta_int() {
        xi = xl = xn;
        pick_next(q);
    }
    void delta_conf(std::list<event_t> const &xb) {
        xj = xb[0].x;
        xi = xl = xn;
        pick_next(q);
    }
    void delta_ext(double e, std::list<event_t> const &xb) {
        double f1 = A[i][i] * xi + A[i][j] * xj;
        xj = xb[0].x;
        double f2 = A[i][i] * (xi + e * f1) + A[i][j] * xj;
        xi += e * (f1 + f2) / 2.0;
        double dx = q - fabs(xi - xl);
        if (dx < 0.0) {
            xn = xi;
            h = 0.0;
        } else {
            pick_next(dx);
        }
    }
    void output_func(std::list<event_t> &y) {
        event_t yy;
        yy.k = i;
        yy.x = xn;
        y.push_back(yy);
    }


  private:
    void pick_next(double D) {
        double f1 = A[i][i] * xi + A[i][j] * xj;
        if (f1 == 0) {
            xn = xi;
            h = adevs_inf<double>();
            return;
        }
        D = (f1 > 0.0) ? D : -D;
        double f2 = A[i][i] * (xi + D) + A[i][j] * xj;
        if (f1 * f2 < 0.0) {
            D = -f1 * D / (f2 - f1);
        }
        xn = xi + D;
        h = fabs(2 * D / (f1 + f2));
    }
    double const q;
    int const i, j;
    double xj, xi, xl, h, xn;
};

class report : public Atomic<event_t> {
  public:
    report() : Atomic<event_t>(), t(0.0) {
        x[0] = x0[0];
        x[1] = x0[1];
        err[0] = err[1] = 0.0;
        std::cout << t << " " << x[0] << " " << x[1] << " 0 0 " << x[0] << " " << x[1] << std::endl;
    }
    void delta_int() { t += ta(); }
    void delta_ext(double e, std::list<event_t> const &xb) {
        // Calculate solution
        t += e;
        double x1 = x0[0] * exp(A[0][0] * t) +
                    x0[1] * A[0][1] * (exp(A[0][0] * t) - exp(A[1][1] * t)) / (A[0][0] - A[1][1]);
        double x2 = exp(A[1][1] * t) * x0[1];
        for (size_t i = 0; i < xb.size(); i++) {
            int k = xb[i].k;
            x[k] = xb[i].x;
            if (k == 0) {
                err[0] = fabs(x[k] - x1);
            } else if (k == 1) {
                err[1] = fabs(x[k] - x2);
            }
        }
        std::cout << t << " " << x[0] << " " << x[1] << " " << err[0] << " " << err[1] << " " << x1
                  << " " << x2 << std::endl;
    }
    void delta_conf(std::list<event_t> const &xb) { delta_ext(ta(), xb); }
    void output_func(std::list<event_t> &) {}
    double ta() { return adevs_inf<double>(); }


  private:
    double t;
    double x[2];
    double err[2];
};

int main(int argc, char** argv) {
    double q = 0.01;
    SimpleDigraph<event_t>* dig = new SimpleDigraph<event_t>();
    qss1* x0 = new qss1(q, 0);
    qss1* x1 = new qss1(q, 1);
    report* r = new report();
    dig->add(x0);
    dig->add(x1);
    dig->add(r);
    //dig->couple(x0,x1);
    dig->couple(x1, x0);
    dig->couple(x0, r);
    dig->couple(x1, r);
    Simulator<event_t>* sim = new Simulator<event_t>(dig);
    while (sim->nextEventTime() < 10.0) {
        sim->execNextEvent();
    }
    delete sim;
    delete dig;
    return 0;
}
