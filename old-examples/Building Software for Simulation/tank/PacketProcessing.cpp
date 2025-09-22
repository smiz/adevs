#include "PacketProcessing.h"
#include <cmath>

// using namespace adevs;

PacketProcessing::PacketProcessing()
    : Atomic<SimEvent>(),
      processing_time(0.0016),
      sigma(DBL_MAX),
      interrupt(false) {}

void PacketProcessing::delta_int() {
    q.pop_front();
    if (q.empty()) {
        sigma = DBL_MAX;
    } else {
        sigma = processing_time;
    }
}

void PacketProcessing::delta_ext(double e, std::list<SimEvent> const &xb) {
    // If we are not interrupted and are processing a packet, then
    // reduce the time remaining to finish with that packet
    if (!interrupt && !q.empty()) {
        sigma -= e;
    }
    // Process input events
    for (std::list<SimEvent>::const_iterator iter = xb.begin(); iter != xb.end();
         iter++) {
        if ((*iter).getType() == SIM_PACKET) {
            q.push_back((*iter).simPacket());
        } else if ((*iter).getType() == SIM_INTERRUPT) {
            interrupt = !interrupt;
        }
    }
    // If we are idle and there are more packets, then start
    // processing the next one
    if (sigma == DBL_MAX && !q.empty()) {
        sigma = processing_time;
    }
}

void PacketProcessing::delta_conf(std::list<SimEvent> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void PacketProcessing::output_func(std::list<SimEvent> &yb) {
    // Set the motor on times from the data in the completed packet
    assert(!q.empty());
    assert(!interrupt);
    SimMotorOnTime on_time;
    on_time.left = fabs(q.front().left_power) * 255.0;
    on_time.right = fabs(q.front().right_power) * 255.0;
    on_time.reverse_left = q.front().left_power < 0.0;
    on_time.reverse_right = q.front().right_power < 0.0;
    yb.push_back(SimEvent(on_time));
}

double PacketProcessing::ta() {
    if (interrupt) {
        return DBL_MAX;  // No work while interrupted
    } else {
        return sigma;  // Otherwise continue processing the packet
    }
}
