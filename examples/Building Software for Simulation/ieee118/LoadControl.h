#ifndef LoadControl_h_
#define LoadControl_h_
#include <map>
#include "ElectricalData.h"
#include "adevs/adevs.h"

class LoadControl : public adevs::Atomic<adevs::PortValue<BasicEvent*>> {
  public:
    static int const load_change;
    static int const sample_arrive;
    static int const sample_setting;

    LoadControl(ElectricalData* data, int freq_steps, double K = 5.0);
    void delta_int();
    void output_func(adevs::Bag<adevs::PortValue<BasicEvent*>> &yb);
    double ta();
    void delta_ext(double, adevs::Bag<adevs::PortValue<BasicEvent*>> const &);
    void delta_conf(adevs::Bag<adevs::PortValue<BasicEvent*>> const &);


  private:
    double const FreqTol, freq_threshold, K;
    std::map<unsigned, double> fdata;
    double adjustment;
    double const max_adjust;
    bool init, signal;
};

#endif
