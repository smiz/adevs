#ifndef _gcd_h_
#define _gcd_h_
#include "adevs/adevs.h"
#include "counter.h"
#include "delay.h"
#include "genr.h"

using pin_t = adevs::pin_t;
using Coupled = adevs::Coupled<ObjectPtr>;

class gcd : public Coupled {
  public:
    pin_t const in;
    pin_t const out;
    pin_t const signal;
    pin_t const start;
    pin_t const stop;

    gcd(std::vector<double> const &pattern, double dt, int iterations, bool active = false)
        : Coupled() {
        auto g = std::shared_ptr<genr>(new genr(pattern, iterations, active));
        auto d = std::shared_ptr<delay>(new delay(dt));
        auto c = std::make_shared<counter>();
        build(g, c, d);
    }
    gcd(double period, double dt, int iterations, bool active = false) : Coupled() {
        auto g = std::shared_ptr<genr>(new genr(period, iterations, active));
        auto d = std::shared_ptr<delay>(new delay(dt));
        auto c = std::make_shared<counter>();
        build(g, c, d);
    }
    ~gcd() {}

  private:
    void build(std::shared_ptr<genr> g, std::shared_ptr<counter> c, std::shared_ptr<delay> d) {
        add_atomic(g);
        add_atomic(d);
        add_atomic(c);
        create_coupling(in, d->in);
        create_coupling(start, g->start);
        create_coupling(stop, g->stop);
        create_coupling(g->signal, signal);
        create_coupling(d->out, out);
        create_coupling(d->out, c->in);
        create_coupling(c->in, c);
        create_coupling(d->in, d);
        create_coupling(g->stop, g);
        create_coupling(g->start, g);
    }
};

#endif
