#include "QueueBus.h"
#include <cstdlib>
#include <iostream>
using namespace adevs;
using namespace std;

int const QueueBus::load = 1;

QueueBus::QueueBus(double packets_per_second)
    : Atomic<PortValue<BasicEvent*>>(),
      ttg(DBL_MAX),
      delay(1.0 / packets_per_second) {}

void QueueBus::schedule_next_packet() {
    ttg = delay;
}

void QueueBus::delta_int() {
    q.pop_front();
    if (q.empty()) {
        ttg = DBL_MAX;
    } else {
        schedule_next_packet();
    }
}

void QueueBus::delta_ext(double e, Bag<PortValue<BasicEvent*>> const &xb) {
    if (!q.empty()) {
        ttg -= e;
    }
    for (Bag<PortValue<BasicEvent*>>::const_iterator iter = xb.begin();
         iter != xb.end(); iter++) {
        if (q.empty()) {
            schedule_next_packet();
        }
        packet_t pkt;
        pkt.e = ((*iter).value);
        if (pkt.e != NULL) {
            pkt.e = pkt.e->clone();
        }
        // Goes out on the same port that it arrived from
        pkt.out_port = ((*iter).port);
        q.push_back(pkt);
    }
}

void QueueBus::delta_conf(Bag<PortValue<BasicEvent*>> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

double QueueBus::ta() {
    return ttg;
}

void QueueBus::gc_output(Bag<PortValue<BasicEvent*>> &gb) {
    for (Bag<PortValue<BasicEvent*>>::const_iterator iter = gb.begin();
         iter != gb.end(); iter++) {
        if ((*iter).value != NULL) {
            delete (*iter).value;
        }
    }
}

void QueueBus::output_func(Bag<PortValue<BasicEvent*>> &yb) {
    assert(!q.empty());
    PortValue<BasicEvent*> y;
    y.value = q.front().e;
    if (y.value != NULL) {
        y.value = y.value->clone();
    }
    y.port = q.front().out_port;
    yb.insert(y);
}
