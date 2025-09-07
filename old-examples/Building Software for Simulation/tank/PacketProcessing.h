#ifndef PACKET_PROCESSING_H_
#define PACKET_PROCESSING_H_
#include <list>
#include "SimEvents.h"
#include "adevs/adevs.h"
// This is the model of the computer's packet processing code.
// Input events must have the type SIM_PACKET or SIM_INTERRUPT.
// Output events have have the type SIM_MOTOR_ON_TIME.
class PacketProcessing : public adevs::Atomic<SimEvent> {
  public:
    PacketProcessing();
    // State transition functions
    void delta_int();
    void delta_ext(double e, std::list<SimEvent> const &xb);
    void delta_conf(std::list<SimEvent> const &xb);
    // Output function
    void output_func(std::list<SimEvent> &yb);
    // Time advance function
    double ta();


  private:
    // Computer time needed to process one packet
    double const processing_time;
    // Time to process the next packet
    double sigma;
    // Are we interrupted?
    bool interrupt;
    // FIFO queue holding packets that need processing
    std::list<SimPacket> q;
};

#endif
