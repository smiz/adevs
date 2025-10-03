#ifndef GenrFail_h_
#define GenrFail_h_
#include "ElectricalData.h"
#include "adevs/adevs.h"

class GenrFail : public adevs::Atomic<adevs::PortValue<BasicEvent*>> {
  public:
    static int const genr_fail;

    GenrFail(unsigned which, double time_to_drop, ElectricalData* data);
    void delta_int();
    void output_func(std::list<adevs::PortValue<BasicEvent*>> &yb);
    double ta();
    void delta_ext(double, std::list<adevs::PortValue<BasicEvent*>> const &) {}
    void delta_conf(std::list<adevs::PortValue<BasicEvent*>> const &) {}


  private:
    unsigned which;
    double time_to_drop;
    ElectricalData* data;
};

#endif
