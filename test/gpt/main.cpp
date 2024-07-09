#include <cstdlib>
#include <iostream>
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
    adevs::Digraph<job> model;
    genr* gnr = new genr(g);
    transd* trnsd = new transd(t);
    proc* prc = new proc(p);
    /// Add the components to the digraph
    model.add(gnr);
    model.add(trnsd);
    model.add(prc);
    /// Establish component coupling
    model.couple(gnr, gnr->out, trnsd, trnsd->ariv);
    model.couple(gnr, gnr->out, prc, prc->in);
    model.couple(prc, prc->out, trnsd, trnsd->solved);
    model.couple(trnsd, trnsd->out, gnr, gnr->stop);
    /// Create a simulator for the model and run it until
    /// the model passivates.
    adevs::Simulator<PortValue> sim(&model);
    while (sim.nextEventTime() < DBL_MAX) {
        sim.execNextEvent();
    }
    /// Done!
    return 0;
}
