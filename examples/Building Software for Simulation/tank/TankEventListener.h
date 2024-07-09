#ifndef TankEventListener_h
#define TankEventListener_h
#include <fstream>
#include "SimEventListener.h"
#include "SimEvents.h"
#include "Tank.h"

class TankEventListener : public SimEventListener {
  public:
    TankEventListener(Tank const* tank)
        : SimEventListener(),
          tank(tank),
          fout("current.dat"),
          E(0.0),   // Accumulated energy starts at zero
          tl(0.0),  // First sample is at time zero
          il(tank->getPhysics()->leftMotorCurrent()),  // i_l(0)
          ir(tank->getPhysics()->rightMotorCurrent())  // i_r(0)
    {
        fout << tl << " " << il << " " << ir << std::endl;
    }
    // Listener does nothing with output events
    void outputEvent(ModelInput, double) {}
    // This method is invoked when an atomic component changes state
    void stateChange(AtomicModel* model, double t) {
        // If this is the model of the tank's physics
        if (model == tank->getPhysics()) {
            // Get the current and motor resistance
            double Rm = tank->getPhysics()->getMotorOhms();
            // Update the enery dissipated in the motors
            E += (t - tl) * (il * il * Rm + ir * ir * Rm);
            // Remember the last sample
            il = tank->getPhysics()->leftMotorCurrent();
            ir = tank->getPhysics()->rightMotorCurrent();
            tl = t;
            fout << tl << " " << il << " " << ir << std::endl;
        }
    }
    // Get the power dissipated in the left and right motors
    double getPowerLost() const { return E / tl; }

  private:
    Tank const* tank;
    std::ofstream fout;
    double E, tl, il, ir;
};

#endif
