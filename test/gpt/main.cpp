#include <cstdlib>
#include <iostream>
#include <memory>
#include "adevs/adevs.h"
#include "genr.h"
#include "job.h"
#include "proc.h"
#include "transd.h"

using namespace std;

int main() {
    /// Get experiment parameters
    double g, p, t;
    cout << "Genr period: ";
    cin >> g;
    cout << "Proc time: ";
    cin >> p;
    cout << "Observation time: ";
    cin >> t;

    /// Create and connect the atomic components using a digraph model.
    shared_ptr<adevs::Digraph<job>> model = make_shared<adevs::Digraph<job>>();
    shared_ptr<genr> gnr = make_shared<genr>(g);
    shared_ptr<transd> trnsd = make_shared<transd>(t);
    shared_ptr<proc> prc = make_shared<proc>(p);

    /// Add the components to the digraph
    model->add(gnr);
    model->add(trnsd);
    model->add(prc);

    /// Establish component coupling
    model->couple(gnr, gnr->out, trnsd, trnsd->ariv);
    model->couple(gnr, gnr->out, prc, prc->in);
    model->couple(prc, prc->out, trnsd, trnsd->solved);
    model->couple(trnsd, trnsd->out, gnr, gnr->stop);

    /// Create a simulator for the model and run it until the model is passive.
    adevs::Simulator<PortValue> sim(model);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }

    return 0;
}
