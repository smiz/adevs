#ifndef _gcd_h_
#define _gcd_h_
#include "adevs/adevs.h"
#include "counter.h"
#include "delay.h"
#include "genr.h"

class gcd {
  public:
    adevs::pin_t in;
    adevs::pin_t out;
    adevs::pin_t signal;
    adevs::pin_t start;
    adevs::pin_t stop;

    gcd(adevs::Graph<ObjectPtr>& graph, std::vector<double> const &pattern, double dt, int iterations,
        bool active = false) {
        auto g = std::shared_ptr<genr>(new genr(pattern, iterations, active));
        auto d = std::shared_ptr<delay>(new delay(dt));
        auto c = std::make_shared<counter>();
        build(graph, g, c, d);
    }
    gcd(adevs::Graph<ObjectPtr>& graph, double period, double dt, int iterations, bool active = false) {
        auto g = std::shared_ptr<genr>(new genr(period, iterations, active));
        auto d = std::shared_ptr<delay>(new delay(dt));
        auto c = std::make_shared<counter>();
        build(graph, g, c, d);
    }
    ~gcd() {}

  private:
    void build(
          adevs::Graph<ObjectPtr>& graph,
          std::shared_ptr<genr> g,
          std::shared_ptr<counter> c,
          std::shared_ptr<delay> d) {
        g->signal = graph.add_pin();
        g->start = graph.add_pin();
        g->stop = graph.add_pin();
        c->in = graph.add_pin();
        d->in = graph.add_pin();
        d->out = graph.add_pin();
        in = graph.add_pin();
        out = graph.add_pin();
        start = graph.add_pin();
        stop = graph.add_pin();
        signal = graph.add_pin();
        graph.add_atomic(g);
        graph.add_atomic(d);
        graph.add_atomic(c);
        graph.connect(in, d->in);
        graph.connect(start, g->start);
        graph.connect(stop, g->stop);
        graph.connect(g->signal, signal);
        graph.connect(d->out, out);
        graph.connect(d->out, c->in);
        graph.connect(c->in,c);
        graph.connect(d->in,d);
        graph.connect(g->stop,g);
        graph.connect(g->start,g);
    }
};

#endif
