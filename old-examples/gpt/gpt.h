#ifndef _gpt_h_
#define _gpt_h_

#include <cstdlib>
#include <iostream>

#include "adevs/adevs.h"
#include "generator.h"
#include "job.h"
#include "processor.h"
#include "transducer.h"

using namespace std;


struct Arguments {
    double generator_period;
    double processor_time;
    double observation_time;
};


int gpt(Arguments args) {

    // Create and connect the atomic components using a digraph model.
    shared_ptr<adevs::Digraph<Job>> model = make_shared<adevs::Digraph<Job>>();

    shared_ptr<Generator> generator = make_shared<Generator>(args.generator_period);
    shared_ptr<Transducer> transducer = make_shared<Transducer>(args.processor_time);
    shared_ptr<Processor> processor = make_shared<Processor>(args.observation_time);

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


#endif
