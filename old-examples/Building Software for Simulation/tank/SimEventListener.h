#ifndef SimEventListener_h_
#define SimEventListener_h_
#include "SimEvents.h"
#include "adevs/adevs.h"

/// Typedefs for adevs
typedef adevs::Atomic<SimEvent> AtomicModel;
typedef adevs::Devs<SimEvent> BasicModel;
typedef adevs::Event<SimEvent> ModelOutput;
typedef adevs::Event<SimEvent> ModelInput;
typedef adevs::Simulator<SimEvent> Simulator;
typedef std::list<ModelInput> ModelInputBag;

/**
 * Class for listening to simulation events.
 */
class SimEventListener : public adevs::EventListener<SimEvent> {
  public:
    void outputEvent(ModelOutput x, double t) = 0;
    void stateChange(AtomicModel* model, double t, void* state) {
        stateChange(model, t);
    }
    virtual void stateChange(AtomicModel* model, double t) = 0;
};

#endif
