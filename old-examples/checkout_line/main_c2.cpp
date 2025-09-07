#include <iostream>

#include "Clerk2.h"
#include "Generator.h"
#include "Observer.h"


using namespace adevs;


int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "Need an input file and output file!" << std::endl;
        return 1;
    }

    // Create the model
    std::shared_ptr<adevs::Digraph<std::shared_ptr<Customer>>> store =
        std::make_shared<adevs::Digraph<std::shared_ptr<Customer>>>();

    std::shared_ptr<Clerk2> clerk = std::make_shared<Clerk2>();
    std::shared_ptr<Generator> generator = std::make_shared<Generator>(argv[1]);
    std::shared_ptr<Observer> observer = std::make_shared<Observer>(argv[2]);

    store->add(clerk);
    store->add(generator);
    store->add(observer);

    store->couple(generator, generator->arrive, clerk, clerk->arrive);
    store->couple(clerk, clerk->depart, observer, observer->departed);

    // Run the simulation
    Simulator<PortValue<std::shared_ptr<Customer>>> simulator(store);
    while (simulator.nextEventTime() < DBL_MAX) {
        simulator.execNextEvent();
    }
    return 0;
}
