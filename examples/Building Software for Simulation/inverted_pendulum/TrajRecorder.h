#ifndef TrajRecorder_h_
#define TrajRecorder_h_
#include <fstream>
#include <iostream>
#include "CartModel.h"
#include "PIDControl.h"
#include "adevs.h"

class TrajRecorder : public adevs::EventListener<double> {
  public:
    TrajRecorder(CartModel* c, adevs::Hybrid<double>* h)
        : c(c), h(h), fy("y.dat"), fs("s.dat") {}
    void stateChange(adevs::Atomic<double>* model, double t) {
        if (model == h) {
            fs << t << " " << c->angle(h->getState()) << std::endl;
        }
    }
    void outputEvent(adevs::Event<double> x, double t) {
        if (x.model == h) {
            fy << t << " " << x.value * RAD_TO_DEG << std::endl;
        }
    }
    ~TrajRecorder() {
        fy.close();
        fs.close();
    }

  private:
    CartModel* c;
    adevs::Hybrid<double>* h;
    std::ofstream fy, fs;
};

#endif
