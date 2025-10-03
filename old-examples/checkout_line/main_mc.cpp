#include <iostream>

#include "Clerk.h"
#include "Clerk2.h"
#include "Customer.h"
#include "Generator.h"
#include "MultiClerk.h"
#include "Observer.h"
#include "adevs/adevs.h"


// using namespace adevs;


int main(int argc, char** argv) {

    if (argc != 3) {
        std::cout << "Need an input and output file!" << std::endl;
        return 1;
    }

    std::shared_ptr<adevs::Digraph<std::shared_ptr<Customer>>> store =
        std::make_shared<adevs::Digraph<std::shared_ptr<Customer>>>();

    std::shared_ptr<MultiClerk> clerk = std::make_shared<MultiClerk>();
    std::shared_ptr<Generator> generator = std::make_shared<Generator>(argv[1]);
    std::shared_ptr<Observer> observer = std::make_shared<Observer>(argv[2]);

    store->add(clerk);
    store->add(generator);
    store->add(observer);

    store->couple(generator, generator->arrive, clerk, clerk->arrive);
    store->couple(clerk, clerk->depart, observer, observer->departed);

    Simulator<EventType> simulator(store);

    while (simulator.nextEventTime() < 100.0) {
        simulator.execNextEvent();
    }
    return 0;
}
