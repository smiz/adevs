#ifndef GenrFail_h_
#define GenrFail_h_
#include "ElectricalData.h"
#include "adevs/adevs.h"

class GenrFail : public adevs::Atomic<adevs::PortValue<BasicEvent*>> {
  public:
    static int const genr_fail;

    GenrFail(unsigned which, double time_to_drop, ElectricalData* data);
    void delta_int();
    void output_func(adevs::Bag<adevs::PortValue<BasicEvent*>> &yb);
    double ta();
    void delta_ext(double, adevs::Bag<adevs::PortValue<BasicEvent*>> const &) {}
    void delta_conf(adevs::Bag<adevs::PortValue<BasicEvent*>> const &) {}
    void gc_output(adevs::Bag<adevs::PortValue<BasicEvent*>> &);

  private:
    unsigned which;
    double time_to_drop;
    ElectricalData* data;
};

#endif
