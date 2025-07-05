#ifndef _gcd_h_
#define _gcd_h_
#include "adevs/adevs.h"
#include "counter.h"
#include "delay.h"
#include "genr.h"

class gcd: public adevs::Coupled<ObjectPtr> {
  public:
    const adevs::pin_t in;
    const adevs::pin_t out;
    const adevs::pin_t signal;
    const adevs::pin_t start;
    const adevs::pin_t stop;

    gcd(std::vector<double> const &pattern, double dt, int iterations,
        bool active = false):adevs::Coupled<ObjectPtr>() {
        auto g = std::shared_ptr<genr>(new genr(pattern, iterations, active));
        auto d = std::shared_ptr<delay>(new delay(dt));
        auto c = std::make_shared<counter>();
        build(g, c, d);
    }
    gcd(double period, double dt, int iterations, bool active = false):
      adevs::Coupled<ObjectPtr>() {
        auto g = std::shared_ptr<genr>(new genr(period, iterations, active));
        auto d = std::shared_ptr<delay>(new delay(dt));
        auto c = std::make_shared<counter>();
        build(g, c, d);
    }
    ~gcd() {}

  private:
    void build(
          std::shared_ptr<genr> g,
          std::shared_ptr<counter> c,
          std::shared_ptr<delay> d) {
        add_atomic(g);
        add_atomic(d);
        add_atomic(c);
        create_coupling(in, d->in);
        create_coupling(start, g->start);
        create_coupling(stop, g->stop);
        create_coupling(g->signal, signal);
        create_coupling(d->out, out);
        create_coupling(d->out, c->in);
        create_coupling(c->in,c);
        create_coupling(d->in,d);
        create_coupling(g->stop,g);
        create_coupling(g->start,g);
    }
};

#endif
