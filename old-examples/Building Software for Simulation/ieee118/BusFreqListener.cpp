#include "BusFreqListener.h"
using namespace std;
using namespace adevs;

BusFreqListener::BusFreqListener(ElectricalModel* model, double cint,
                                 string model_name)
    : EventListener<PortValue<BasicEvent*>>(),
      cint(cint),
      fout(string(model_name + "_all_freq.dat").c_str()),
      t_last_record(0.0),
      src(model) {
    freq = new double[src->getElectricalData()->getNodeCount()];
    src->getBusFreqs(freq);
}

void BusFreqListener::stateChange(Atomic<PortValue<BasicEvent*>>* model,
                                  double t) {
    if (t - t_last_record >= cint) {
        t_last_record = t;
        fout << t << " ";
        src->getBusFreqs(freq);
        for (unsigned i = 0; i < src->getElectricalData()->getNodeCount();
             i++) {
            fout << freq[i] / 6.28 << " ";  // in the pu system
        }
        fout << endl;
    }
}

BusFreqListener::~BusFreqListener() {
    delete[] freq;
    fout.close();
}
