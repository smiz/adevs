#ifndef _BusFreqListener_h_
#define _BusFreqListener_h_
#include <fstream>
#include "ElectricalModel.h"
#include "adevs.h"

/**
 * This listener records frequency at all of the busses, load and generation.
 */
class BusFreqListener
    : public adevs::EventListener<adevs::PortValue<BasicEvent*>> {
  public:
    /**
		 * The listener will record bus frequencies from the
		 * provided ElectricalModel model. Values are recorded at most
		 * at intervals cint. Data is stored in model_name+"_all_freq.dat".
		 * The output data is in Hertz and measures deviation from nominal.
		 */
    BusFreqListener(ElectricalModel* model, double cint = 1E-1,
                    std::string model_name = "");
    void outputEvent(adevs::Event<adevs::PortValue<BasicEvent*>>, double) {}
    void stateChange(adevs::Atomic<adevs::PortValue<BasicEvent*>>* model,
                     double t);
    ~BusFreqListener();

  private:
    double const cint;
    std::ofstream fout;
    double* freq;
    double t_last_record;
    ElectricalModel* src;
};

#endif
