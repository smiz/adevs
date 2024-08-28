#ifndef _gcd_h_
#define _gcd_h_
#include "adevs/adevs.h"
#include "counter.h"
#include "delay.h"
#include "genr.h"

class gcd : public adevs::Digraph<object*> {
  public:
    static int const in;
    static int const out;
    static int const signal;
    static int const start;
    static int const stop;

    gcd(std::vector<double> const &pattern, double dt, int iterations,
        bool active = false)
        : adevs::Digraph<object*>() {
        genr* g = new genr(pattern, iterations, active);
        delay* d = new delay(dt);
        counter* c = new counter;
        build(g, c, d);
    }
    gcd(double period, double dt, int iterations, bool active = false)
        : adevs::Digraph<object*>() {
        genr* g = new genr(period, iterations, active);
        delay* d = new delay(dt);
        counter* c = new counter;
        build(g, c, d);
    }
    ~gcd() {}

  private:
    void build(genr* g, counter* c, delay* d) {
        add(g);
        add(d);
        add(c);
        couple(this, in, d, d->in);
        couple(this, start, g, g->start);
        couple(this, stop, g, g->stop);
        couple(g, g->signal, this, signal);
        couple(d, d->out, this, out);
        couple(d, d->out, c, c->in);
    }
};

int const gcd::in(0);
int const gcd::out(1);
int const gcd::start(2);
int const gcd::stop(3);
int const gcd::signal(4);

#endif
