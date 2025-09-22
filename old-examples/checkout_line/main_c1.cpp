#include <iostream>

#include "Clerk.h"
#include "Generator.h"
#include "Observer.h"


int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "Need input and output files!" << std::endl;
        return 1;
    }

    // Create a digraph model whose components use PortValue<Customer*>
    // objects as input and output objects.
    std::shared_ptr<adevs::Digraph<std::shared_ptr<Customer>>> store =
        std::make_shared<adevs::Digraph<std::shared_ptr<Customer>>>();

    // Create and add the component models
    std::shared_ptr<Clerk> clerk = std::make_shared<Clerk>();
    std::shared_ptr<Generator> generator = std::make_shared<Generator>(argv[1]);
    std::shared_ptr<Observer> observer = std::make_shared<Observer>(argv[2]);

    store->add(clerk);
    store->add(generator);
    store->add(observer);

    // Couple the components
    store->couple(generator, generator->arrive, clerk, clerk->arrive);
    store->couple(clerk, clerk->depart, observer, observer->departed);

    // Create a simulator and run until its done
    adevs::Simulator<EventType> simulator(store);

    while (simulator.nextEventTime() < DBL_MAX) {
        simulator.execNextEvent();
    }

    return 0;
}
