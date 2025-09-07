#include <cstdlib>
#include <iostream>
#include <memory>
#include <fstream>
#include "adevs/adevs.h"
#include "genr.h"
#include "job.h"
#include "proc.h"
#include "transd.h"



int main(int argc, char** argv) {
    double g, p, t;
    /// Get experiment parameters
    if (argc == 1) {
        std::cout << "Genr period: ";
        std::cin >> g;
        std::cout << "Proc time: ";
        std::cin >> p;
        std::cout << "Observation time: ";
        std::cin >> t;
    } else {
        std::ifstream fin(argv[1]);
        fin >> g >> p >> t;
        fin.close();
    }

    /// Create and connect the atomic components using a digraph model.
    std::shared_ptr<adevs::Graph<job>> model = std::make_shared<adevs::Graph<job>>();
    std::shared_ptr<genr> gnr = std::make_shared<genr>(g);
    std::shared_ptr<transd> trnsd = std::make_shared<transd>(t);
    std::shared_ptr<proc> prc = std::make_shared<proc>(p);
    /// Add the components to the digraph
    model->add_atomic(gnr);
    model->add_atomic(trnsd);
    model->add_atomic(prc);

    /// Establish component coupling
    model->connect(trnsd->ariv, trnsd);
    model->connect(prc->in, prc);
    model->connect(gnr->stop,gnr);
    model->connect(trnsd->solved,trnsd);
    model->connect(gnr->out, trnsd->ariv);
    model->connect(gnr->out, prc->in);
    model->connect(prc->out, trnsd->solved);
    model->connect(trnsd->out, gnr->stop);

    /// Create a simulator for the model and run it until the model is passive.
    adevs::Simulator<job> sim(model);
    while (sim.nextEventTime() < adevs_inf<double>()) {
        sim.execNextEvent();
    }

    return 0;
}
