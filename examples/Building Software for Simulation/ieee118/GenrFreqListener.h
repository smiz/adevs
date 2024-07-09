#ifndef _GenrFreqListener_h_
#define _GenrFreqListener_h_
#include <fstream>
#include "ElectricalModel.h"
#include "adevs.h"

/**
 * This listener records frequency at the generators.
 */
class GenrFreqListener
    : public adevs::EventListener<adevs::PortValue<BasicEvent*>> {
  public:
    /**
		 * The listener will record generator frequencies from the
		 * provided ElectricalModel model. Values are recorded at most
		 * at intervals cint. Data is stored in model_name+"_freq.dat".
		 * The output data is in Hertz and measures deviation from nominal.
		 */
    GenrFreqListener(ElectricalModel* model, double cint = 1E-1,
                     std::string model_name = "");
    void outputEvent(adevs::Event<adevs::PortValue<BasicEvent*>>, double) {}
    void stateChange(adevs::Atomic<adevs::PortValue<BasicEvent*>>* model,
                     double t);
    ~GenrFreqListener();

  private:
    double const cint;
    std::ofstream fout;
    double t_last;
    ElectricalModel* src;
};

#endif
