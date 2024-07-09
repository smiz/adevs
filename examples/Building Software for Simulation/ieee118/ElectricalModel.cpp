#include "ElectricalModel.h"
using namespace adevs;

static double const ERR_TOL = 1E-6;
static double const H_MAX = 1E-2;

ElectricalModel::ElectricalModel(ElectricalModelEqns* eqns)
    : Hybrid<PortValue<BasicEvent*>>(
          eqns, new rk_45<PortValue<BasicEvent*>>(eqns, ERR_TOL, H_MAX),
          new linear_event_locator<PortValue<BasicEvent*>>(eqns, ERR_TOL)),
      update_vi(true),
      eqns(eqns) {}

double ElectricalModel::getErrTol() const {
    return ERR_TOL;
}

double ElectricalModel::getBusAngle(unsigned bus) {
    return arg(getVoltage(bus));
}

void ElectricalModel::do_vi_update() {
    if (update_vi) {
        eqns->updateVoltageAndInjCurrent(getState());
    }
    update_vi = false;
}

Complex ElectricalModel::getVoltage(unsigned bus) {
    do_vi_update();
    return eqns->getVoltage(bus);
}

Complex ElectricalModel::getInjCurrent(unsigned bus) {
    do_vi_update();
    return eqns->getInjCurrent(bus);
}

ElectricalModel::~ElectricalModel() {}
