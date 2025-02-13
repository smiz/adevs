#include <cstdlib>
#include <iostream>

#include "adevs/adevs.h"
#include "generator.h"
#include "job.h"
#include "processor.h"
#include "transducer.h"

using namespace std;


int main() {

    // Get experiment parameters
    double g, p, t;

    cout << "Generator period: ";
    cin >> g;

    cout << "Processor time: ";
    cin >> p;

    cout << "Observation time: ";
    cin >> t;

    // Create and connect the atomic components using a digraph model.
    shared_ptr<adevs::Digraph<Job>> model = make_shared<adevs::Digraph<Job>>();

    shared_ptr<Generator> generator = make_shared<Generator>(g);
    shared_ptr<Transducer> transducer = make_shared<Transducer>(t);
    shared_ptr<Processor> processor = make_shared<Processor>(p);

    // Add the components to the digraph
    model->add(generator);
    model->add(transducer);
    model->add(processor);

    // Establish component coupling
    model->couple(generator, generator->out, transducer, transducer->ariv);
    model->couple(generator, generator->out, processor, processor->in);
    model->couple(processor, processor->out, transducer, transducer->solved);
    model->couple(transducer, transducer->out, generator, generator->stop);

    adevs::Simulator<PortValue> simulator(model);
    while (simulator.nextEventTime() < DBL_MAX) {
        simulator.execNextEvent();
    }

    return 0;
}
