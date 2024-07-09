#ifndef __CARTPID_H__
#define __CARTPID_H__
#include <omnetpp.h>
#include "CartModel.h"
#include "PIDControl.h"
#include "TrajRecorder.h"

class CartPID : public cSimpleModule,  // From OMNET++
                public adevs::EventListener<double> {
  public:
    void outputEvent(adevs::Event<double> y, double t);
    void stateChange(adevs::Atomic<double>*, double) {}
    ~CartPID();

  protected:
    // OMNET++ method for model initialization
    void initialize();
    // Method for processing OMNET++ events
    void handleMessage(cMessage* msg);
    void registerDSAP(char const* gate);

  private:
    PIDControl* pid;               // The PID controller
    adevs::Hybrid<double>* hysim;  // Model of the cart and arm
    TrajRecorder* traj;            // Listener for recording the cart trajectory
    adevs::SimpleDigraph<double>* top_model;  // Holds the PID and cart
    adevs::Simulator<double>* sim;            // Simulator for our model
    cMessage self_event;  // OMNET++ event for our internal events
    adevs::Bag<adevs::Event<double>> xbag;  // Bag for OMNET++ inputs
    std::ofstream sensor_rx_strm;           // Records receipt of sensor msgs
};

#endif
