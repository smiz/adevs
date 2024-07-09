#include "GenrFail.h"
using namespace adevs;

int const GenrFail::genr_fail = 0;

GenrFail::GenrFail(unsigned which, double time_to_drop, ElectricalData* data)
    : Atomic<PortValue<BasicEvent*>>(),
      which(which),
      time_to_drop(time_to_drop),
      data(data) {}

double GenrFail::ta() {
    return time_to_drop;
}

void GenrFail::delta_int() {
    time_to_drop = DBL_MAX;
}

void GenrFail::output_func(Bag<PortValue<BasicEvent*>> &yb) {
    GenrFailEvent* f = new GenrFailEvent(data->getGenrs()[which]);
    yb.insert(PortValue<BasicEvent*>(genr_fail, f));
}

void GenrFail::gc_output(Bag<PortValue<BasicEvent*>> &gb) {
    Bag<PortValue<BasicEvent*>>::iterator iter = gb.begin();
    for (; iter != gb.end(); iter++) {
        delete (*iter).value;
    }
}
