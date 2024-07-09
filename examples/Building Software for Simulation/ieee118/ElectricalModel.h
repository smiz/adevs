#ifndef _electrical_model_h_
#define _electrical_model_h_
#include <map>
#include "ElectricalData.h"
#include "ElectricalModelEqns.h"
#include "adevs.h"
#include "adevs_hybrid.h"
#include "events.h"

/**
 * This is an atomic model to contain and simulate an ElectricalModelEqns object.
 * Coupling should be done using the ports from the ElectricalModelEqns class.
 */
class ElectricalModel : public adevs::Hybrid<adevs::PortValue<BasicEvent*>> {
  public:
    /**
		 * The ElectricalModelEqns are adopted by the ElectricalModel and
		 * are destroyed when it is.
		 */
    ElectricalModel(ElectricalModelEqns* eqns);
    /// Get the complex voltage at the specified bus
    Complex getVoltage(unsigned bus);
    /// Get the complex current injected into the specified bus
    Complex getInjCurrent(unsigned bus);
    /// Get the voltage angle at a bus.
    double getBusAngle(unsigned bus);
    /// Get the frequency in radians at the specified generator.
    double getGenrFreq(unsigned genr_number) {
        return eqns->getGenrFreq(genr_number, getState());
    }
    /// Get the mechanical power at the specified generator
    double getMechPower(unsigned genr_number) {
        return eqns->getMechPower(genr_number, getState());
    }
    /// Get the bus frequencies
    void getBusFreqs(double* freq) { eqns->getBusFreqs(getState(), freq); }
    /// Is the generator offline?
    bool genrOffLine(unsigned genr_number) {
        return eqns->genrOffLine(genr_number);
    }
    /// Get the electrical data associated with this model
    ElectricalData* getElectricalData() { return eqns->getElectricalData(); }
    /// Get the dynamic equations
    ElectricalModelEqns* getDynamicModel() { return eqns; }
    /// Get the error tolerance for the numerical integrator
    double getErrTol() const;
    /// Destructor
    ~ElectricalModel();

  private:
    bool update_vi;
    // Electrical data for the model
    ElectricalModelEqns* eqns;
    void do_vi_update();
};

#endif
