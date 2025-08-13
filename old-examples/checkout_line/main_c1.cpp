#include <iostream>

#include "Clerk.h"
#include "Generator.h"
#include "Observer.h"

using namespace std;


int main(int argc, char** argv) {

    if (argc != 3) {
        cout << "Need input and output files!" << endl;
        return 1;
    }

    // Create a digraph model whose components use PortValue<Customer*>
    // objects as input and output objects.
    shared_ptr<adevs::Digraph<shared_ptr<Customer>>> store =
        make_shared<adevs::Digraph<shared_ptr<Customer>>>();

    // Create and add the component models
    shared_ptr<Clerk> clerk = make_shared<Clerk>();
    shared_ptr<Generator> generator = make_shared<Generator>(argv[1]);
    shared_ptr<Observer> observer = make_shared<Observer>(argv[2]);

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
