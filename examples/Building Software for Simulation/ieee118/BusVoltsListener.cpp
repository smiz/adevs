#include "BusVoltsListener.h"
using namespace std;
using namespace adevs;

BusVoltsListener::BusVoltsListener(ElectricalModel* model, double cint,
                                   string model_name)
    : EventListener<PortValue<BasicEvent*>>(),
      cint(cint),
      fout(string(model_name + "_volt.dat").c_str()),
      t_last_record(0.0),
      src(model) {
    fout << 0.0;
    for (unsigned i = 0; i < src->getElectricalData()->getNodeCount(); i++) {
        fout << " " << abs(src->getVoltage(i));
    }
    fout << endl;
}

void BusVoltsListener::stateChange(Atomic<PortValue<BasicEvent*>>* model,
                                   double t) {
    if (t - t_last_record >= cint) {
        t_last_record = t;
        fout << t;
        for (unsigned i = 0; i < src->getElectricalData()->getNodeCount();
             i++) {
            fout << " " << abs(src->getVoltage(i));
        }
        fout << endl;
    }
}

BusVoltsListener::~BusVoltsListener() {
    fout.close();
}
