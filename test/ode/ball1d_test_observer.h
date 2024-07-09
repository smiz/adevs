#ifndef ball1d_test_observer
#define ball1d_test_observer
#include <cassert>
#include <iostream>
#include "adevs/adevs.h"
#include "check_ball1d_solution.h"

/**
 * This class checks the output values of the ball1d model for
 * accuracy relative to the known trajectory and the precision
 * of bounce event times.
 */
class BallObserver : public adevs::EventListener<adevs::PortValue<double>> {
  public:
    BallObserver(adevs::Devs<adevs::PortValue<double>>* model)
        : adevs::EventListener<adevs::PortValue<double>>(),
          model(model),
          output_count(0) {}

    void outputEvent(adevs::Event<adevs::PortValue<double>> e, double t) {
        if (e.model == model) {
            output_count++;
            double h = e.value.value;
            assert(ball1d_soln_ok(t, h));
        }
    }

    int getOutputCount() const { return output_count; }

  private:
    adevs::Devs<adevs::PortValue<double>>* model;
    int output_count;
};

#endif
