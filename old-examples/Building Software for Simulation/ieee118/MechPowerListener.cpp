#include "MechPowerListener.h"

// using namespace adevs;

MechPowerListener::MechPowerListener(ElectricalModel* model, double cint,
                                     std::string model_name)
    : EventListener<PortValue<BasicEvent*>>(),
      cint(cint),
      fout(std::string(model_name + "_pm.dat").c_str()),
      t_last(0.0),
      src(model) {}

void MechPowerListener::stateChange(Atomic<PortValue<BasicEvent*>>* model,
                                    double t) {
    if (t - t_last >= cint) {
        t_last = t;
        fout << t << " ";
        unsigned genrs = src->getElectricalData()->getGenrCount();
        for (unsigned i = 0; i < genrs; i++) {
            fout << src->getMechPower(i) << " ";
        }
        fout << std::endl;
    }
}

MechPowerListener::~MechPowerListener() {
    fout.close();
}
