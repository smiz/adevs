#include "LoadControl.h"
#include <map>
#include <vector>
#include "ElectricalData.h"
#include "events.h"

// using namespace adevs;

int const LoadControl::load_change = 0;
int const LoadControl::sample_setting = 1;
int const LoadControl::sample_arrive = 2;

LoadControl::LoadControl(ElectricalData* data, int freq_steps, double K)
    : Atomic<PortValue<BasicEvent*>>(),
      FreqTol(data->getGenrParams(data->getGenrs().front()).FreqTol * 6.28),
      freq_threshold(FreqTol / freq_steps),
      K(K),
      adjustment(0.0),
      max_adjust(0.3) {
    init = true;
    signal = false;
    for (std::vectorunsigned>::const_iterator iter = data->getGenrs().begin();
         iter != data->getGenrs().end(); iter++) {
        fdata[*iter] = 0.0;
    }
}

double LoadControl::ta() {
    if (init || signal) {
        return 0.0;
    }
    return DBL_MAX;
}

void LoadControl::delta_int() {
    init = signal = false;
}

void LoadControl::delta_conf(std::list<PortValue<BasicEvent*>> const &xb) {
    delta_int();
    delta_ext(0.0, xb);
}

void LoadControl::delta_ext(double e, std::list<PortValue<BasicEvent*>> const &xb) {
    std::list<PortValue<BasicEvent*>>::const_iterator iter = xb.begin();
    for (; iter != xb.end(); iter++) {
        GenrSampleEvent* measurement =
            dynamic_cast<GenrSampleEvent*>((*iter).value);
        if (measurement->freqBreakerOpen()) {
            fdata.erase(measurement->getBusID());
        } else {
            fdata[measurement->getBusID()] = measurement->getRotorSpeed();
        }
    }
    // Compute adjustment fraction
    double modified_adjustment = 0.0;
    // Get average frequency
    map<unsigned, double>::iterator fiter = fdata.begin();
    for (; fiter != fdata.end(); fiter++) {
        modified_adjustment += (*fiter).second;
    }
    modified_adjustment /= fdata.size();
    // Normalize to a percentage and apply the gain
    modified_adjustment *= (K / FreqTol);
    // Signal?
    if (!(fabs(modified_adjustment) <= max_adjust)) {
        if (modified_adjustment > 0.0) {
            modified_adjustment = max_adjust;
        } else {
            modified_adjustment = -max_adjust;
        }
    }
    signal = adjustment != modified_adjustment;
    adjustment = modified_adjustment;
    assert(fabs(adjustment) <= max_adjust);
}

void LoadControl::output_func(std::list<PortValue<BasicEvent*>> &yb) {
    if (signal) {
        LoadAdjustEvent* load_adjust = new LoadAdjustEvent(adjustment);
        yb.push_back(PortValue<BasicEvent*>(load_change, load_adjust));
    }
    if (init) {
        map<unsigned, double>::iterator fiter = fdata.begin();
        for (; fiter != fdata.end(); fiter++) {
            GenrSampleEvent* s = new GenrSampleEvent((*fiter).first);
            s->setRotorSpeedSensitivity(freq_threshold);
            s->outputImmediately(false);
            s->oneShot(false);
            yb.push_back(PortValue<BasicEvent*>(sample_setting, s));
        }
    }
}
