#ifndef QUEUE_BUS_H
#define QUEUE_BUS_H
#include <list>
#include "adevs.h"
#include "events.h"

class QueueBus : public adevs::Atomic<adevs::PortValue<BasicEvent*>> {
  public:
    static int const load;
    /// Create a Bus with a throughput
    QueueBus(double packets_per_second);
    void delta_int();
    void delta_ext(double e,
                   adevs::Bag<adevs::PortValue<BasicEvent*>> const &xb);
    void delta_conf(adevs::Bag<adevs::PortValue<BasicEvent*>> const &xb);
    double ta();
    void output_func(adevs::Bag<adevs::PortValue<BasicEvent*>> &yb);
    void gc_output(adevs::Bag<adevs::PortValue<BasicEvent*>> &);
    size_t getPacketCount() const { return q.size(); }

  private:
    struct packet_t {
        BasicEvent* e;
        int out_port;
    };
    std::list<packet_t> q;
    double ttg, delay;
    void schedule_next_packet();
};

#endif
